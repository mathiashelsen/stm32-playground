package main

import (
	"flag"
	"log"
	"net/http"
	"strconv"
)

const BUFSIZE = 512

var( serial tty
 buf = make([]byte, BUFSIZE)
)

func main() {
	flag.Parse()

	var err error
	baud, _ := strconv.Atoi(flag.Arg(1))
	serial, err = OpenTTY(flag.Arg(0), baud)
	if err != nil {
		log.Fatal(err)
	}

	go func(){
		for{
			//log.Println("waiting for frame")
			serial.ReadFull(buf)
		}
	}()

	http.HandleFunc("/", rootHandler)
	http.HandleFunc("/screen.svg", screenHandler)

	http.ListenAndServe(":4000", nil)
}


