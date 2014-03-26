package main

import (
	"flag"
	"fmt"
	"log"
	"net/http"
	"strconv"
	"sync"
)

const (
	BUFSIZE = 512
	MAGIC   = 0xEF9A1387
)

var (
	serial   tty
	data     = make([]byte, BUFSIZE)
	state    State
	buffer   = make([]byte, BUFSIZE)
	stateBuf State
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
		if serial.readInt() != MAGIC {
			// Print it so that we notice when things go suspicious.
			// A few frame syncs after the cable has been touched is OK,
			// much more is not.
			log.Println("Frame syncing...")
		}
		for serial.readInt() != MAGIC {
			//skip
		}

		dataLock.Lock()

		stateBuf.Samples = serial.readInt()
		stateBuf.TimeBase = serial.readInt()
		stateBuf.TrigLev = serial.readInt()
		serial.ReadFull(buffer)

		buffer, data = data, buffer
		stateBuf, state = state, stateBuf
		fmt.Println(state)

		dataLock.Unlock()
	}
}
