#!/bin/python
# NAME: Tejas Bhat, Aayush Patel
# EMAIL: tejasgbhat@gmail.com, aayushpatel108@ucla.edu
# ID: 705199445, 105111196

import sys
import csv 

def reset_file_pointer():
	file_pointer.seek(0)

def str_int(line_string, i):
	return int(line[i])

start_range = 12
end_range = 27
inode_information = {}
block_information = {}
directory_information = {}

group_types = {'GROUP', 'BFREE', 'SUPERBLOCK', 'INDIRECT', 'IFREE'}
offsets = [0, 12, 268, 65804]
block_levels = ['BLOCK', 'INDIRECT BLOCK', 'DOUBLE INDIRECT BLOCK', 'TRIPLE INDIRECT BLOCK']

file_format = 'rb' # binary reading
file_pointer = open(sys.argv[1], file_format)
file_reader = csv.reader(file_pointer)

for line in file_reader:
	if line[0] == 'INDIRECT': 
		block_information[str_int(line, 5)] = ['used', 0, [], [], [], []] 
	elif line[0] == 'SUPERBLOCK' or line[0] == 'BFREE' or line[0] == 'IFREE':
		line_info = str_int(line, 1)
		if line[0] == 'SUPERBLOCK':
			size_inode = str_int(line, 4)
			ro_inode = str_int(line, 7)
			num_blocks = line_info
			size_of_block = str_int(line, 3)
		elif line[0] == 'BFREE':
			block_information[line_info] = ['free', 0, [], [], [], []] 
		elif line[0] == 'IFREE':
			if line_info != 2: 
				inode_information[line_info] = ['free', 0, 0, None, [], []] 
			else:
				inode_information[line_info] = ['free', 0, 0, line_info, [], []] 
	elif line[0] == 'GROUP':
		total_inodes = str_int(line, 3)
		reserved_offset_block = str_int(line, 8) + size_inode * total_inodes / size_of_block

reset_file_pointer()
file_reader = csv.reader(file_pointer)
for line in file_reader:
	if line[0] == 'INODE':
		inode = str_int(line, 1)
		for i in range(start_range, end_range):
			if(str_int(line, i) == 0):
				continue
			block = str_int(line, i)
			if block in block_information and block_information[block][0] == 'free':
				block_information[block][0] = 'free_alloc'
			if block in block_information and i < 24:
				block_information[block][2].append(inode)
			if block in block_information and i == 24:
				block_information[block][3].append(inode)
			if block in block_information and i == 25:
				block_information[block][4].append(inode)
			if block in block_information and i == 26:
				block_information[block][5].append(inode)
			if block not in block_information:
				if block < reserved_offset_block:
					if i < 24:
						block_information[block] = ['reserved', offsets[0], [inode], [], [], []]
					elif i == 24:
						block_information[block] = ['reserved', offsets[1], [], [inode], [], []]
					elif i == 25:
						block_information[block] = ['reserved', offsets[2], [], [], [inode], []]
					elif i == 26:
						block_information[block] = ['reserved', offsets[3], [], [], [], [inode]]
				else:
					if i < 24:
						block_information[block] = ['used', offsets[0], [inode], [], [], []]
					elif i == 24:
						block_information[block] = ['used', offsets[1], [], [inode], [], []]
					elif i == 25:
						block_information[block] = ['used', offsets[2], [], [], [inode], []]
					elif i == 26:
						block_information[block] = ['used', offsets[3], [], [], [], [inode]]
		curr_inode = str_int(line, 6)
		if inode not in inode_information:
			if inode != 2:
				inode_information[inode] = ['alloc', 0, curr_inode, None, [], []]
			else:
				inode_information[inode] = ['alloc', 0, curr_inode, 2, [], []]
		else:
			inode_information[inode][2] = curr_inode
			inode_information[inode][0] = 'free_alloc'
			

reset_file_pointer()
file_reader = csv.reader(file_pointer)
for line in file_reader:
	if line[0] == 'DIRENT':
		parent_inode = str_int(line, 1)
		reference_name = line[6]
		inode_reference = str_int(line, 3)
		if reference_name == "'..'":
			directory_information[inode_reference] = [reference_name,parent_inode]
		elif reference_name == "'.'":
			directory_information[inode_reference] = [reference_name, parent_inode]
		else:
			if inode_reference == 2 or inode_reference not in inode_information: 
				inode_information[inode_reference] = ['unalloc', 0, 0, parent_inode, [], []]  	
			else:
				inode_information[inode_reference][3] = parent_inode

			if parent_inode not in inode_information:
				if parent_inode != 2:
					inode_information[parent_inode] = ['unalloc', 0, 0, None, [reference_name], [inode_reference]]  
				else:
					inode_information[parent_inode] = ['unalloc', 0, 0, 2, [reference_name], [inode_reference]]  
			else:
				inode_information[parent_inode][4].append(reference_name)
				inode_information[parent_inode][5].append(inode_reference)
		inode_information[inode_reference][1] = inode_information[inode_reference][1] + 1

gen = (entry for entry in range(reserved_offset_block, num_blocks) if entry not in block_information)
for x in gen: 
	str_x = str(x)
	print 'UNREFERENCED BLOCK ' + str_x

for entry in block_information:
	if block_information[entry][0] == 'free_alloc':
		print 'ALLOCATED BLOCK ' + str(entry) + ' ON FREELIST'
	if entry > num_blocks or entry < 1:
		for j in range(0, 4):
			for i in range(0, len(block_information[entry][j+2])):
				print 'INVALID ' + block_levels[j] + ' ' + str(entry) + ' IN INODE ' + str(block_information[entry][j+2][i]) + ' AT OFFSET ' + str(offsets[j])
	
	if block_information[entry][0] == 'reserved':
		for j in range(0, 4):
			for i in range(0, len(block_information[entry][j+2])):
				print 'RESERVED ' + block_levels[j] + ' ' + str(entry) + ' IN INODE ' + str(block_information[entry][j+2][i]) + ' AT OFFSET ' + str(offsets[j])
	
	summation = 0
	for index in range(2, 6):
		summation = summation + len(block_information[entry][index])
	if 1 < summation:
		start_range = 0
		for j in range(start_range, start_range + 4):
			for i in range(0, len(block_information[entry][j+2])):
				print 'DUPLICATE ' + block_levels[j] + ' ' + str(entry) + ' IN INODE ' + str(block_information[entry][j+2][i]) + ' AT OFFSET ' + str(offsets[j])


gen = (entry for entry in range(ro_inode, total_inodes) if entry not in inode_information)
for x in gen: 
	str_x = str(x)
	print 'UNALLOCATED INODE ' + str_x + ' NOT ON FREELIST'

for entry in directory_information:
	if entry != 2 and directory_information[entry][0] == "'..'":
		if entry != inode_information[directory_information[entry][1]][3] and directory_information[directory_information[entry][1]][1] != 2:
			dir_inode_num = str(directory_information[entry][1])
			print 'DIRECTORY INODE ' + dir_inode_num + ' NAME \'..\' LINK TO INODE ' + str(entry) + ' SHOULD BE ' + str(inode_information[directory_information[entry][1]][3])
	elif directory_information[entry][1] != entry:
		if directory_information[entry][0] == "'.'":
			print 'DIRECTORY INODE ' + str(entry) + ' NAME \'.\' LINK TO INODE ' + str(directory_information[entry][1]) + ' SHOULD BE ' + str(entry)
 
inode_type = ' '
for entry in inode_information:
	start_inodes = 0
	end_inodes = len(inode_information[entry][5])
	for i in range(start_inodes, end_inodes):
		if inode_information[entry][5][i] < 1: 
			inode_type = 'INVALID'
		elif inode_information[entry][5][i] > total_inodes:
			inode_type = 'INVALID'
		elif inode_information[inode_information[entry][5][i]][0] == 'unalloc' or inode_information[inode_information[entry][5][i]][0] == 'free':
			inode_type = 'UNALLOCATED'
		if inode_type != ' ':
			print 'DIRECTORY INODE ' + str(entry) + ' NAME ' + inode_information[entry][4][i] + ' ' + inode_type + ' INODE ' + str(inode_information[entry][5][i])
		inode_type = ' '
	if inode_information[entry][1] != inode_information[entry][2]:
		if inode_information[entry][0] == 'alloc':
			print 'INODE ' + str(entry) + ' HAS ' + str(inode_information[entry][1]) + ' LINKS BUT LINKCOUNT IS ' + str(inode_information[entry][2])
	if inode_information[entry][0] == 'free_alloc':
		print 'ALLOCATED INODE ' + str(entry) + ' ON FREELIST'
