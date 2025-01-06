package base

import "unsafe"

const (
	VERSION    = 0
	KB         = 1024
	MB         = 1024 * 1024
	GB         = 1024 * 1024 * 1024
	TB         = 1024 * 1024 * 1024 * 1024
	BLOCK_SIZE = 4 * KB // in KB, 4KB
	UNIT       = KB

	DATA_NODE_SIZE = BLOCK_SIZE - 2*int(unsafe.Sizeof(int(0))) //黑白节点中数据实体大小
)
