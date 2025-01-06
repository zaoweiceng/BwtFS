package tool

import (
	"crypto/sha256"
	"os"

	"example.com/fs/V1/base"
	"example.com/fs/V1/bwt"
)

func CheckSum(b []byte) [32]byte {
	hash := sha256.New()
	hash.Write(b)
	return [32]byte(hash.Sum(nil))
}

func Verify(filename string) bool {
	INFO("verify file: " + filename)
	data, err := ReadFile(filename, 0)
	if err != nil {
		ERROR(err, "Verify")
		return false
	}
	fm := bwt.FileMetadata{}
	fm.FromBinary(data)
	fm.File_size = uint64(getFilesize(filename) / base.MB)
	fi := bwt.FileInfo{}
	file_info, err := ReadFile(filename, fm.End_block)
	if err != nil {
		ERROR(err, "Verify")
		return false
	}
	fi.FromBinary(file_info)
	if CheckSum(fm.ToBinary()) == fi.Checksum {
		INFO("verify success!")
		return true
	}
	WARNING("verify failed! Checksum not match, file may be modified!")
	return false
}

func getFilesize(filename string) int64 {
	file, _ := os.Open(filename)
	defer file.Close()
	fi, _ := file.Stat()
	return fi.Size()
}
