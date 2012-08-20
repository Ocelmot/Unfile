#!/usr/bin/env python

import argparse
import hashlib
import os
import shutil
import socket		#At some point, I'm going to wish I used twisted for this
import struct
import sys

#Things to fix:
#Filename conflicts when copying files into /unique
#Server, host, and cutoff should have cmdline overrides

#Hardcoded globals
#BAD BAD BAD - make opts, specifiable, etc.
host = 'isaacnet.dyndns.org'
port = 3000
cutoff = 2

def getFileSeparator():
	"""If this is Windows, return \\. If this is not Windows, return /"""
	if sys.platform == 'win32':
		return "\\"
	else:
		return "/"

def transmitHashes(hashes, subcommand):
	"""Send the hashed file contents to the server"""
	counts = []
	global host, port
	server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	server.connect((host, port))
	
	for hash in hashes:
		#Transmit the hash to the server
		binary = hash.decode('hex')
		message = struct.pack(">ibb", len(binary), 1, int(subcommand))
		server.sendall(message + binary)
		
		#Receive the hash count back from the server
		header = server.recv(6)
		bytecount = server.recv(4)
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
	
def main():
	"""The main function of the script"""
	#Run the argparser to get command line input
	parser = argparse.ArgumentParser(description="Locates unique files on your computer to be backed up")
	parser.add_argument("directory", metavar="DIRECTORY", type=str, help="The directory to check for unique files, default is current directory", nargs="?", default=os.getcwd())
	parser.add_argument("-c", "--copy", dest="copy", action="store_true", help="Copy all detected unique files to a specified folder", default=False)
	parser.add_argument("--copy-dir", dest="copydir", help="Directory where unique files will be copied to", default=os.path.join(os.getcwd(), 'unique'))
	parser.add_argument("-u", "--update", dest="update", action="store_true", help="Update the hash database on the server before checking", default=False)
	args = parser.parse_args()
	
	#Build an array of all hashed file names
	hashes = []
	hashdict = {}
	directory = os.path.abspath(args.directory)
	print "Scanning and hashing files in " + directory + "..."
	for dirpath, dirnames, filenames in os.walk(directory):
		for file in filenames:
			fullname = dirpath + getFileSeparator() + file
			fileObject = open(fullname, 'rb')
			filedata = fileObject.read()
			fileObject.close()
			hashed = hashlib.sha1(filedata).hexdigest()
			#Testing code - check duplicate files
			#if hashed in hashes:
			#	print "Hash " + hashed + " is a duplicate!"
			hashes.append(hashed)
			hashdict.update({hashed:fullname})
	
	#Transmit the data, either updating it on the server or not updating it
	counts = []
	uniqueHashes = []
	print "Transmitting hashes to the server, retrieving uniqueness data..."
	try:
		if args.update:
			counts = transmitHashes(hashes, '0')
		else:
			counts = transmitHashes(hashes, '1')
		uniqueHashes = getUniqueHashes(hashes, counts)
	except:
		print "Error: Unable to transmit hashes to server"
		print "Reported error was: "
		print str(sys.exc_info())
		return
		
	#Now, do dictionary lookup to find the unique files!
	uniques = []
	for hash in uniqueHashes:
		try:
			unique = hashdict[hash]
			if unique != None and unique != "":
				uniques.append(unique)
		except:
			pass
			
	#Print the unique files- make fancier output later
	if len(uniques) == 0:
		print "No unique files have been detected, exiting"
		return	
	print "Detected the following unique files: "
	for unique in uniques:
		print "  " + unique
	
	#Copy the unique files somewhere, if we're told to do so
	if args.copy:
		copydir = os.path.abspath(args.copydir)
		print "Copying files to " + copydir + "..."
		if not os.path.exists(copydir):
			os.mkdir(copydir)
		for unique in uniques:
			shutil.copy2(unique, copydir)

if __name__ == '__main__':
	main()