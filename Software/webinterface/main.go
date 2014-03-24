package main

import (
	"flag"
	"fmt"
	"log"
)

func main() {
	flag.Parse()

	tty, err := OpenTTY(flag.Arg(0), 9600)
	if err != nil {
		log.Fatal(err)
	}

	for {
		buf := make([]byte, 1024)
		n, err := tty.Read(buf)
		if err != nil {
			log.Fatal(err)
		}
		fmt.Print(buf[:n])
	}
}
