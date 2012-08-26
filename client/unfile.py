#!/usr/bin/env python

import argparse
import hashlib
import os
import math
import multiprocessing
import random
import shutil
import socket
import struct
import sys

#Things to fix:
#Filename conflicts when copying files into /unique?
#Cutoff should not be hardcoded

#Hardcoded globals
processMultiplier = 2	#Used to determine how many processes are spawned
cutoff = 2				#The cutoff used for determing unique files

def getFileSeparator():
	"""If this is Windows, return \\. If this is not Windows, return /"""
	if sys.platform == 'win32':
		return "\\"
	else:
		return "/"
		
def readN(conn, n):
	"""Reads n bytes from socket server conv in a robust manner, code from Chrys"""
	buffer = ""
	while len(buffer) < n:
		buffer += conn.recv(n - len(buffer))
	return buffer
	
def splitList(seq, num):
	"""Yields num roughly even chunks from list seq, code courtesy of Stack Overflow"""
	avg = len(seq) / float(num)
	output = []
	last = 0.0
	while last < len(seq):
		output.append(seq[int(last):int(last + avg)])
		last += avg
	return output
	
def transmitHashes(host, port, hashes, subcommand):
	"""Send the hashed file contents to the server (host, port), using a subcommand"""
	counts = []
	server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	server.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
	server.connect((host, port))
	
	#Wait for 0,2 command from the server before transmitting
	go = False
	while not go:
		bytereply = readN(server, 6)
		if ord(bytereply[4]) == 0 and ord(bytereply[5]) == 2:
			go = True
	
	#Compute an array of messages to transmit
	messages = []
	for hash in hashes:
		binary = hash.decode('hex')
		message = struct.pack(">ibb", len(binary), 1, int(subcommand))
		message += binary
		messages.append(message)
		
	#Transmit the messages in groups of 62
	transmission = ""
	count = 0
	for message in messages:
		count += 1
		if count >= 63:
			server.sendall(transmission)
			transmission = ""
			count = 0
		transmission += message
		
	#If we have a pending transmission after the loop, send it
	if transmission != "":
		server.sendall(transmission)
	
	#Receive replies (hash counts) back from server
	for i in range(len(hashes)):
		reply = readN(server, 10)
		bytecount = reply[6:10]
		count = struct.unpack(">i", bytecount)[0]
		counts.append(count)		
		
	#Close the link to the server
	message = struct.pack(">ibb", 0, 0, 0)
	server.sendall(message)
	server.close()
	
	#Return the counts
	return counts
	
def getUniqueHashes(hashes, counts):
	"""Calculate which hashes qualify as unique based on an array of counts"""
	#Make this server-calculated or command-line overriden later
	global cutoff
	if len(hashes) != len(counts):
		print "Error: count and hash databases are invalid!"
		return []
		
	#Loop through both arrays, find the hashes whose corresponding count is greater than cutoff
	uniques = []
	for i in range(len(hashes)):
		hash = hashes[i]
		count = counts[i]
		if count <= cutoff:
			uniques.append(hash)
			
	#Return the array of unique hashes
	return uniques
	
def multiProcessUnfile(fullnames, server, port, subcommand, copydir, queue, lock, number):
	"""Function that, when spawned as a process, creates hashes, uploads, and checks for unique files"""
	
	#Build an array and dictionary of all hashes and file names
	hashes = []
	hashdict = {}
	for fullname in fullnames:
		fileObject = open(fullname, 'rb')
		filedata = fileObject.read()
		fileObject.close()
		hashed = hashlib.sha1(filedata).hexdigest()
		hashes.append(hashed)
		hashdict.update({hashed:fullname})
		
	#Transmit that data to the server, either updating or not updating depending on subcommand
	counts = []
	uniqueHashes = []
	try:
		counts = transmitHashes(server, port, hashes, subcommand)
		uniqueHashes = getUniqueHashes(hashes, counts)
	except:
		raise
		
	#Now, do dictionary lookup to find the unique files!
	uniques = []
	for hash in uniqueHashes:
		try:
			unique = hashdict[hash]
			if unique != None and unique != "":
				uniques.append(unique)
		except:
			pass
			
	#Output unique files
	lock.acquire()
	for unique in uniques:
		print "  " + unique
	lock.release()
	
	#Copy unique files, if we have a valid copying directory
	if copydir != "":
		if not os.path.exists(copydir):
			os.mkdir(copydir)
		for unique in uniques:
			shutil.copy2(unique, copydir)
	
def createProcesses(directory, server, port, subcommand, copydir):
	"""Creates the subprocesses used by unfile to do everything"""
	processes = []
	
	#The maximum number of processes to allow at a time
	global processMultiplier
	numProcesses = multiprocessing.cpu_count() * processMultiplier
	
	#Build a list of all files in directory using os.walk
	fullfiles = []
	for dirpath, dirnames, filenames in os.walk(directory):
		for filename in filenames:
			fullname = dirpath + getFileSeparator() + filename
			fullfiles.append(fullname)
	
	#Now, create processes - we should never have too many processes, but if we do- it handles it!
	random.shuffle(fullfiles)
	lock = multiprocessing.Lock()
	filesArray = splitList(fullfiles, numProcesses)
	for i in range(len(filesArray)):
		#If we don't have too many processes already, create more
		if len(processes) <= numProcesses:
			queue = multiprocessing.Queue()
			filenames = filesArray[i]
			process = multiprocessing.Process(target=multiProcessUnfile, args=(filenames, server, port, subcommand, copydir, queue, lock, i))
			process.start()
			processes.append([process, queue])
	
		#If we have too many processes, join them and then remove them - if this code ever actually triggers we have a problem
		else:
			print "Error: Something has gone wrong! Spawning too many processes, oh no!"
			for process, queue in processes:
				process.join()
				processes.remove([process, queue])
	
	#Now, finish up all remaining processes
	if len(processes) > 0:
		for process, queue in processes:
			process.join()
			
def main():
	"""The main function of the script"""
	#Run the argparser to get command line input
	parser = argparse.ArgumentParser(description="Locates unique files on your computer to be backed up")
	parser.add_argument("directory", metavar="DIRECTORY", type=str, help="Directory that will be scanned for unique files", nargs="?", default=os.getcwd())
	parser.add_argument("-c", "--copy", dest="copy", action="store_true", help="Copy all detected unique files to a specified folder", default=False)
	parser.add_argument("--copy-dir", dest="copydir", help="Directory where unique files will be copied to", default=os.path.join(os.getcwd(), 'unique'))
	parser.add_argument("-p", "--port", dest="port", help="Server port over which to attempt connection", default="3000")
	parser.add_argument("-s", "--server", dest="server", help="Address of the unfile server to connect to", default="isaacnet.dyndns.org")
	parser.add_argument("-u", "--update", dest="update", action="store_true", help="Update the hash database on the server before checking", default=False)
	args = parser.parse_args()
	
	#Parse arguments to get server, port, copydir, and subcommand data
	server = args.server
	subcommand = 1
	copydir = ""
	if args.update:
		subcommand = 0
	if args.copy:
		copydir = os.path.abspath(args.copydir)
	try:
		port = int(args.port)
	except:
		print "Error: invalid port specified, defaulting to 3000"
		port = 3000
	
	#Use multiprocessing to spawn subprocesses which will work in each directory
	directory = os.path.abspath(args.directory)
	print "Scanning and hashing files in " + directory + "..."
	try:
		createProcesses(directory, server, port, subcommand, copydir)
	except:
		print "Error: " + str(sys.exc_info())
		return

if __name__ == '__main__':
	main()