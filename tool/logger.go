package tool

import (
	"log"
)

func ERROR(err error, func_name string) {
	if err != nil {
		log.Println("\033[31m ERROR: \033[0m", err, " FuncName: ", func_name)
	}
}

func WARNING(warning string) {
	log.Println("\033[33m WARN:  \033[0m", warning)
}

func INFO(info string) {
	log.Println("\033[34m INFO:  \033[0m", info)
}

func DEBUG(debug string) {
	log.Println("\033[32m DEBUG: \033[0m", debug)
}
