package main

// Serves SVG image of scope screen

import (
	"github.com/ajstarks/svgo"
	"net/http"
)

var (
	screenW, screenH = 1024, 256
	gridDiv          = 32
)

func screenHandler(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "image/svg+xml")
	w.Header().Set("Cache-control", "No-Cache")

	canvas := svg.New(w)
	canvas.Start(screenW, screenH)

	// Grid
	for i := 0; i < screenW; i += gridDiv {
		canvas.Line(i, 0, i, screenH, "stroke:grey;")
	}
	for i := 0; i < screenH; i += gridDiv {
		canvas.Line(0, i, screenW, i, "stroke:grey;")
	}
	canvas.Rect(0, 0, screenW, screenH, "stroke:black; fill:none; stroke-width:4")

	// Data
	x := make([]int, BUFSIZE)
	y := make([]int, BUFSIZE)
	for i := range buffer {
		x[i] = i
		y[i] = screenH - int(buffer[i]/16) // 14-bit to 8-bit
	}
	canvas.Polyline(x, y, "stroke:blue; fill:none; stroke-width:3")

	canvas.End()
}
