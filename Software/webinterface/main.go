package main

import (
	"flag"
	"log"
	"net/http"
	"strconv"
	"sync"
)

const BUFSIZE = 512

var (
	serial tty
	data   = make([]byte, BUFSIZE)
	buffer = make([]byte, BUFSIZE)
	dataLock sync.Mutex
)

func main() {
	flag.Parse()

	var err error
	baud, _ := strconv.Atoi(flag.Arg(1))
	serial, err = OpenTTY(flag.Arg(0), baud)
	if err != nil {
		log.Fatal(err)
	}

	go func() {
		for {
			serial.ReadFull(buffer)
			dataLock.Lock()
			buffer, data = data, buffer
			dataLock.Unlock()
		}
	}()

	http.HandleFunc("/", rootHandler)
	http.HandleFunc("/screen.svg", screenHandler)

	err = http.ListenAndServe(":4000", nil)
	if err != nil{
		log.Fatal(err)
	}
}
