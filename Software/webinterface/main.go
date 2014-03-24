package main

import (
	"flag"
	"log"
	"net/http"
	"github.com/ajstarks/svgo"
)

var serial tty

func main() {
	flag.Parse()

	var err error
	serial, err = OpenTTY(flag.Arg(0), 9600)
	if err != nil {
		log.Fatal(err)
	}
	//serial.ReadFull(buf)
	for i:=range buf{
		buf[i] = byte(i)
	}

	go func(){
		for{
			serial.ReadFull(buf)
			log.Println("read frame")
		}
	}()

	http.HandleFunc("/", imgHandler)

	http.ListenAndServe(":4000", nil)
}

var	buf = make([]byte, 1024*2)

func imgHandler(w http.ResponseWriter, r*http.Request){

	const S = 1
	W,H := 800, 600
	canvas := svg.New(w)

	w.Header().Set("Content-Type", "image/svg+xml")
    w.Header().Set("Cache-control", "No-Cache")

	canvas.Start(S*W, S*H)


	for i := 1; i<len(buf); i++{
		canvas.Line(i-1, int(buf[i-1]), i, int(buf[i]), "stroke:red")
	}

	canvas.End()
}
