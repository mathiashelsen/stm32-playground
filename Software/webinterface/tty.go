package main

import (
	"errors"
	"log"
)

////cgo CFLAGS: -Wall -Werror
//#include "tty.h"
import "C"
import "unsafe"

type tty int

func OpenTTY(fname string, baud int) (tty, error) {
	fd := C.openTTY(C.CString(fname), C.int(baud))
	if fd < 0 {
		return 0, errors.New(C.GoString(C.TTYErr))
	}
	return tty(fd), nil
}

func (t tty) Read(p []byte) (n int, err error) {
	n = int(C.readTTY(C.int(t), unsafe.Pointer(&p[0]), C.int(len(p))))
	if n < 0 {
		return 0, errors.New("tty read error")
	} else {
		return n, nil
	}
}


func (t tty) ReadFull(p []byte)  {
	N, err := t.Read(p)
	for N < len(p){
		if err != nil{
			log.Fatal(err)
		}
		var n int
		n, err = t.Read(p[N:])
		N += n
	}
}
