package main

import (
	"flag"
	"log"
	"net/http"
	"github.com/ajstarks/svgo"
	"strconv"
	"fmt"
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




func screenHandler(w http.ResponseWriter, r*http.Request){

	const S = 1
	W,H := 512, 512
	canvas := svg.New(w)

	w.Header().Set("Content-Type", "image/svg+xml")
    w.Header().Set("Cache-control", "No-Cache")

	canvas.Start(S*W, S*H)


	for i := 1; i<len(buf); i++{
		canvas.Line(i-1, int(buf[i-1]), i, int(buf[i]), "stroke:red")
	}

	canvas.End()
}



func rootHandler(w http.ResponseWriter, r*http.Request){
	fmt.Fprintln(w, page)
}

const page = `
<!DOCTYPE html>
<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
	<title> Scope </title>
	<style media="all" type="text/css">

		body  { margin-left: 5%; margin-right:5%; font-family: sans-serif; }
		table { border-collapse: collapse; }
		hr    { border-style: none; border-top: 1px solid #CCCCCC; }
		a     { color: #375EAB; text-decoration: none; }
	</style>

</head>

<body>
	
	<h1>Poor man's scope</h1>

	<img src="/screen.svg" />

</body>
</html>
`
