package tool

func GetToken(start_bitmap uint64, bitmaps []byte) string {
	data_bytes := DataToBytes(start_bitmap, bitmaps)
	token := EncodeToken(data_bytes)
	return token
}

func GetDataFromToken(token string) (uint64, []byte) {
	data, _ := DecodeToken(token)
	return BytesToData(data)
}
