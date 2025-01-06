package tool

import (
	"bytes"
	"encoding/base64"
	"unsafe"
)

func EncodeToken(token []byte) string {
	return base64.StdEncoding.EncodeToString(token)
}

func DecodeToken(token string) ([]byte, error) {
	return base64.StdEncoding.DecodeString(token)
}

func DataToBytes(start_bitmap uint64, bitmaps []byte) []byte {
	var buf bytes.Buffer
	buf.Write((*[8]byte)(unsafe.Pointer(&start_bitmap))[:])
	buf.Write(bitmaps)
	return buf.Bytes()
}

func BytesToData(data []byte) (start_bitmap uint64, bitmaps []byte) {
	start_bitmap = *(*uint64)(unsafe.Pointer(&data[0]))
	bitmaps = []byte(data[8:])
	return start_bitmap, bitmaps
}
