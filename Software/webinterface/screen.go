package main

// Serves SVG image of scope screen

import(
	"net/http"
	"github.com/ajstarks/svgo"
)

var(
	screenW, screenH = 512, 512
)

func screenHandler(w http.ResponseWriter, r*http.Request){
	w.Header().Set("Content-Type", "image/svg+xml")
    w.Header().Set("Cache-control", "No-Cache")

	const S = 1
	W,H := 512, 512

	canvas := svg.New(w)
	canvas.Start(S*W, S*H)

	x := make([]int, BUFSIZE)
	y := make([]int, BUFSIZE)
	for i := range buf{
		x[i] = i
		y[i] = int(buf[i])
	}

	canvas.Polyline(x, y, "stroke:red")

	canvas.End()
}



