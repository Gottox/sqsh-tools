#!/usr/bin/python3
import struct
import os
import lzo
import lz4.frame
import zlib
import lzma
import pyzstd
import argparse

parser = argparse.ArgumentParser()

parser.add_argument('--data', type=str, default='abcd')
parser.add_argument('--algorithm', type=str, default='lzo')
parser.add_argument('--out', type=str, default='helper')

args = parser.parse_args()

data = bytes(args.data, 'utf-8')

compressed = None
match args.algorithm:
	case 'zlib':
		compressed = zlib.compress(data)
	case 'lzma':
		compressed = lzma.compress(data)
	case 'lz4':
		compressed = lz4.frame.compress(data)
	case 'zstd':
		compressed = pyzstd.compress(data)
	case 'lzo':
		compressed = lzo.compress(data)
	case _:
		raise ValueError('Unknown algorithm')

match args.out:
	case 'array':
		print(','.join('0x{:02x}'.format(x) for x in compressed))
	case 'helper':
		os.write(1, struct.pack('@Q', len(data)))
		os.write(1, struct.pack('@Q', len(compressed)))
		os.write(1, compressed)
	case 'plain':
		os.write(1, compressed)
