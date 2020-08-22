#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "ext2_fs.h"

int block_size;
int image_fd;
struct ext2_super_block public_superblock;
struct ext2_group_desc group_description;
char filetype;
int num_blocks;

void print_time(int block_time) {
    time_t epoch_time = block_time;
    char* time_string = malloc(sizeof(char) * 30);
    strftime(time_string, 30, "%m/%d/%y %H:%M:%S", gmtime(&epoch_time));
    printf("%s,", time_string);
    free(time_string);
}

unsigned long get_offset(unsigned int block) {
    return (block - 1) * block_size + 1024;
}

void free_blocks() {
    char* bytes = (char*) malloc(block_size);
    pread(image_fd, bytes, block_size, get_offset(group_description.bg_block_bitmap));
    int i;
    int j;
    for (i = 0; i < block_size; i++) {
        for (j = 0; j < 8; j++) {
            if (!(bytes[i] >> j & 1)) {
                fprintf(stdout, "BFREE,%d\n", i*8+j+public_superblock.s_first_data_block);
            }
        }
    }
    free(bytes);
}

void read_directory(unsigned int parent_inode, unsigned int block_num) {
    struct ext2_dir_entry directory_entry;
    int i;
    for (i = 0; i < block_size; i += directory_entry.rec_len) {
        bzero(directory_entry.name, 256);
        pread(image_fd, &directory_entry, sizeof(directory_entry), get_offset(block_num) + i);
        if (directory_entry.inode) {
            bzero(&directory_entry.name[directory_entry.name_len], 256 - directory_entry.name_len);
            fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n", parent_inode, i, directory_entry.inode, directory_entry.rec_len, directory_entry.name_len, directory_entry.name);
        }
    }
}

void direct_entries(char filetype, struct ext2_inode inode, unsigned int inode_num) {
    int i;
    int allocated = 0;
    for (i = 0; i < 12; i++) {
        allocated = inode.i_block[i];
        if (allocated) {
            if (filetype == 'd')
                read_directory(inode_num, inode.i_block[i]);
        }
    }
}

int calculate_base_offset(int level) {
    int result = 12;
    int i;
    int j;
    for (i = 1; i < 3; i++) {
        if (level > i) {
            int place_value = 1;
            for (j = 0; j < i; j++)
                place_value*=256;
            result+=place_value;
        }
    }
    return result;
}

void print_inode_information(uint32_t *block_ptrs, int inode_num, int root_level, int level, int indirect_block_number, int i) {
    if (filetype == 'd' && level == 1) {
        read_directory(inode_num, block_ptrs[i]);
    }
    int base_offset = calculate_base_offset(root_level);
    fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", inode_num, level, base_offset + i, indirect_block_number, block_ptrs[i]);
}

void level_1_indirection(int inode_num, int root_level, int indirect_block_number) {
    uint32_t *blocks = malloc(block_size);
    pread(image_fd, blocks, block_size, get_offset(indirect_block_number));
    int i;
    for (i = 0; i < num_blocks; i++) {
        if (blocks[i])
            print_inode_information(blocks, inode_num, root_level, 1, indirect_block_number, i);
    }
    free(blocks);
}

void level_2_indirection(int inode_num, int root_level, int indirect_block_number) {
    uint32_t *indirect_1_blocks = malloc(block_size);
    pread(image_fd, indirect_1_blocks, block_size, get_offset(indirect_block_number));
    int i;
    for (i = 0; i < num_blocks; i++) {
        if (indirect_1_blocks[i]) {
            print_inode_information(indirect_1_blocks, inode_num, root_level, 2, indirect_block_number, i);
            level_1_indirection(inode_num, root_level, indirect_1_blocks[i]);
        }
    }
    free(indirect_1_blocks);
}


void level_3_indirection(int inode_num, int indirect_block_number) {
    uint32_t *indirect_2_blocks = malloc(block_size);
    pread(image_fd, indirect_2_blocks, block_size, get_offset(indirect_block_number));
    int i;
    for (i = 0; i < num_blocks; i++) {
        if (indirect_2_blocks[i]) {
            print_inode_information(indirect_2_blocks, inode_num, 3, 3, indirect_block_number, i);
            level_2_indirection(inode_num, 3, indirect_2_blocks[i]);
        }
    }
    free(indirect_2_blocks);
}

void set_filetype(struct ext2_inode inode) {
    filetype = '?';
    switch (inode.i_mode & 0xF000) {
        case (0xa000):
            filetype = 's';
            break;
        case (0x8000):
            filetype = 'f';
            break;
        case (0x4000):
            filetype = 'd';
            break;
    }
}


void read_inode(unsigned int inode_table_id, int inode_num) {
    
    num_blocks = (int) block_size / sizeof(uint32_t);
    struct ext2_inode inode;
    pread(image_fd, &inode, sizeof(inode), inode_num * sizeof(inode) + get_offset(inode_table_id));
    inode_num++;
    if (inode.i_mode && inode.i_links_count) {
        set_filetype(inode);
        
        printf("INODE,%d,%c,%o,%d,%d,%d,", inode_num, filetype, inode.i_mode & 0xFFF, inode.i_uid, inode.i_gid, inode.i_links_count);
        print_time(inode.i_ctime);
        print_time(inode.i_mtime);
        print_time(inode.i_atime);
        printf("%d,%d", inode.i_size, (inode.i_blocks / (2 << public_superblock.s_log_block_size))*2);

        int i;
        for (i = 0; i < 15; i++) {
            printf(",%d", inode.i_block[i]);
        }
        printf("\n");
        direct_entries(filetype, inode, inode_num);
        if (inode.i_block[12])
            level_1_indirection(inode_num, 1, inode.i_block[12]);
            if (inode.i_block[13])
            level_2_indirection(inode_num, 2, inode.i_block[13]);
        if (inode.i_block[14])
            level_3_indirection(inode_num, inode.i_block[14]);
    }

}

void print_and_free_inodes() {
    int num_bytes = public_superblock.s_inodes_per_group / 8;
    char* bytes = (char*) malloc(num_bytes);
    pread(image_fd, bytes, num_bytes, get_offset(group_description.bg_inode_bitmap));
    int i, j;
    for (i = 0; i < num_bytes; i++) {
        for (j = 0; j < 8; j++) {
            if (!(1 & bytes[i]>>j)) {
                printf("IFREE,%d\n", i*8+j+1);
            } else {
                read_inode(group_description.bg_inode_table, i*8+j);
            }
        }
    }
    free(bytes);
}

void get_block_group_summary() {
    uint32_t descblock = 1;
    if (block_size == 1024)
        descblock = 2;

    pread(image_fd, &group_description, sizeof(group_description), block_size * descblock);

    printf("GROUP,0,%d,%d,%d,%d,%d,%d,%d\n", public_superblock.s_blocks_count, public_superblock.s_inodes_count, group_description.bg_free_blocks_count, group_description.bg_free_inodes_count, group_description.bg_block_bitmap, group_description.bg_inode_bitmap, group_description.bg_inode_table);

    free_blocks();
    print_and_free_inodes();
}

int main(int argc, char* argv[]) {
    
    if (argc > 2) {
        fprintf(stderr, "Error parsing arguments: too few arguments.\n");
        exit(1);
    } else if (argc > 2) {
        fprintf(stderr, "Error parsing arguments; too many arguments.\n");
        exit(1);
    }
    image_fd = open(argv[1], O_RDONLY);
    if (image_fd < 0) {
        fprintf(stderr, "Error opening image file\n");
        exit(1);
    }
    pread(image_fd, &public_superblock, sizeof(public_superblock), 1024);

    block_size = 1024 << public_superblock.s_log_block_size;
    printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", public_superblock.s_blocks_count, public_superblock.s_inodes_count, block_size, public_superblock.s_inode_size, public_superblock.s_blocks_per_group, public_superblock.s_inodes_per_group, public_superblock.s_first_ino);
    
    get_block_group_summary();

    exit(0);
}

