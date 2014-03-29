package main

import (
	"flag"
	"fmt"
	"log"
	"net/http"
	"strconv"
	"unsafe"
)

const (
	BUFSIZE = 1024
)

var (
	serial tty
	buffer = make([]uint16, BUFSIZE)
)

func main() {
	flag.Parse()

	var err error
	baud, _ := strconv.Atoi(flag.Arg(1))
	serial, err = OpenTTY(flag.Arg(0), baud)
	if err != nil {
		log.Fatal(err)
	}

	go StreamInput()

	http.HandleFunc("/", rootHandler)
	http.HandleFunc("/event", eventHandler)
	http.HandleFunc("/screen.svg", screenHandler)

	err = http.ListenAndServe(":4000", nil)
	if err != nil {
		log.Fatal(err)
	}
}

func StreamInput() {
	for {

		// TODO (proto): not correct: should scan bytes, not int
		//
		// Skip to the beginning of what is presumably a new frame.
		// Once we are in sync we should not have to skip anymore,
		// but in the rare case a bit should fall over, we will re-sync quickly.
		//	if serial.readInt() != MAGIC {
		//		// Print it so that we notice when things go suspicious.
		//		// A few frame syncs after the cable has been touched is OK,
		//		// much more is not.
		//		log.Println("Frame syncing...")
		//	}
		//	for serial.readInt() != MAGIC {
		//		//skip
		//	}

		// TODO (proto) move into state.Read
		//	stateBuf.Samples = serial.readInt()
		//	stateBuf.TimeBase = serial.readInt()
		//	stateBuf.TrigLev = serial.readInt()
		//	stateBuf.SoftGain = serial.readInt()

		header := make([]byte, 2)
		serial.ReadFull(header)
		fmt.Println("Frame starts with", header)
		bytes := (*(*[1<<31 - 1]byte)(unsafe.Pointer(&buffer[0])))[:2*len(buffer)]
		serial.ReadFull(bytes)

	}
}
