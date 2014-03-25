package main

// Serves SVG image of scope screen

import(
	"net/http"
	"github.com/ajstarks/svgo"
)

var(
	screenW, screenH = 512, 256
	gridDiv = 32
)

func screenHandler(w http.ResponseWriter, r*http.Request){
	w.Header().Set("Content-Type", "image/svg+xml")
    w.Header().Set("Cache-control", "No-Cache")

	canvas := svg.New(w)
	canvas.Start(screenW, screenH)

	// Grid
	for i := 0; i < screenW; i+= gridDiv{
		canvas.Line(i, 0, i, screenH, "stroke:grey;")
	}
	for i := 0; i < screenH; i+= gridDiv{
		canvas.Line(0, i, screenW, i, "stroke:grey;")
	}
	canvas.Rect(0, 0, screenW, screenH, "stroke:black; fill:none; stroke-width:3")

	// Data
	x := make([]int, BUFSIZE)
	y := make([]int, BUFSIZE)
	for i := range buf{
		x[i] = i
		y[i] = screenH - int(buf[i])
	}
	canvas.Polyline(x, y, "stroke:blue; fill:none; stroke-width:1")

	canvas.End()
}



