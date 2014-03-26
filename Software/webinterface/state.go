package main

import(
	"fmt"
	"io"
	"strconv"
	"log"
)

const MAGIC = 252730

type State struct{
	Samples int
	TimeBase int
	TrigLev int
}

func NewState(m map[string]interface{})State{
	var s State

	s.Samples = atoi(m["Samples"])
	s.TrigLev = atoi(m["TrigLev"])
	s.TimeBase = atoi(m["TimeBase"])

	return s
}

func(s*State)WriteTo(w io.Writer){
	writeInt(w, MAGIC)
	writeInt(w, s.Samples)
	writeInt(w, s.TimeBase)
	writeInt(w, s.TrigLev)
}

func writeInt(w io.Writer, I int){
	i := uint32(I)
	bytes := []byte{
	byte((i & 0xFF000000) >> 24),
	byte((i & 0x00FF0000) >> 16),
	byte((i & 0x0000FF00) >> 8),
	byte((i & 0x000000FF) >> 0) }
	_, err := w.Write(bytes)
	log.Println("write", bytes)
	if err != nil{
		log.Fatal(err)
	}
}

func atoi(a interface{})int{
	i, err := strconv.Atoi(fmt.Sprint(a))
	if err != nil{log.Println(err)}
	return i
}
