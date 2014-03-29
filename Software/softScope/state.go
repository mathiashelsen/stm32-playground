package main

import (
	"fmt"
	"log"
	"strconv"
)

type State struct {
	Samples  int
	TimeBase int
	TrigLev  int
	SoftGain int
}

func NewState(m map[string]interface{}) State {
	var s State

	s.Samples = atoi(m["Samples"])
	s.TrigLev = atoi(m["TrigLev"])
	s.TimeBase = atoi(m["TimeBase"])
	s.SoftGain = atoi(m["SoftGain"])

	return s
}

func (s *State) WriteTo(w tty) {
	w.writeInt(MAGIC)
	w.writeInt(s.Samples)
	w.writeInt(s.TimeBase)
	w.writeInt(s.TrigLev)
	w.writeInt(s.SoftGain)
}

func atoi(a interface{}) int {
	i, err := strconv.Atoi(fmt.Sprint(a))
	if err != nil {
		log.Println(err)
	}
	return i
}
