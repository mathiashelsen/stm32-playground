package main

import (
	"fmt"
	"log"
	"strconv"
)

const HEADER_WORDS = 8

type Header struct {
	Magic    uint32
	Samples  uint32
	TrigLev  uint32
	padding [HEADER_WORDS-3]uint32
}

//func NewHeader(m map[string]interface{}) Header {
//	var s State
//
//	s.Samples = atoi(m["Samples"])
//	s.TrigLev = atoi(m["TrigLev"])
//	s.TimeBase = atoi(m["TimeBase"])
//	s.SoftGain = atoi(m["SoftGain"])
//
//	return s
//}

func (s *Header) WriteTo(w tty) {
	w.writeInt(s.Magic)
	w.writeInt(s.Samples)
	w.writeInt(s.TrigLev)

	for i:=3; i<HEADER_WORDS; i++{
		w.writeInt(0)
	}
}

func atoi(a interface{}) uint32 {
	i, err := strconv.Atoi(fmt.Sprint(a))
	if err != nil {
		log.Println(err)
	}
	return uint32(i)
}
