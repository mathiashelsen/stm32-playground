package main

// Serves the scope's main page.

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
)

func rootHandler(w http.ResponseWriter, r *http.Request) {
	switch r.Method {
	case "GET":
		fmt.Fprintln(w, page)
	case "PUT":
		state := make(map[string]interface{})
		check(json.NewDecoder(r.Body).Decode(&state))
		fmt.Println("POST:", state)
		s := NewState(state)
		s.WriteTo(serial)
	}
}

func check(err error) {
	if err != nil {
		log.Fatal(err)
	}
}

const page = `
<!DOCTYPE html>
<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
	<title>SoftScope</title>

	<link rel="stylesheet" href="http://yui.yahooapis.com/pure/0.4.2/pure-min.css">
	<style media="all" type="text/css">

		body  { margin-left: 5%; margin-right:5%; font-family: sans-serif; }
		table { border-collapse: collapse; }
		hr    { border-style: none; border-top: 1px solid #CCCCCC; }
		a     { color: #375EAB; text-decoration: none; }
	</style>

	<script>

// wraps document.getElementById, shows error if not found
function elementById(id){
	var elem = document.getElementById(id);
	if (elem == null){
		alert("undefined: " + id);
		return null;
	}
	return elem;
}

function setAttr(id, attr, value){
	var elem = elementById(id);
	if (elem[attr] == null){
		alert("settAttr: undefined: " + elem + "[" + attr + "]");
		return;
	}
	elem[attr] = value;
}

function refresh(){
	setAttr("screen", "src", "/screen.svg?" + Math.random()) // refresh screen.svg, random cachebreaker
}

setInterval(refresh, 200);

function val(id){
	return elementById(id).value
}

function upload(){
	var req = new XMLHttpRequest();
	req.open("PUT", document.URL, false);
	var map = {
		"Samples": val("Samples"),
		"TrigLev": val("TrigLev"),
		"TimeBase": val("TimeBase")};
	req.send(JSON.stringify(map));
}

	</script>

</head>

<body>
	
	<h1><i>Soft</i>Scope</h1>

<div>
	<img id="screen" src="/screen.svg" />
</div>

<div style="padding-top:2em;">
	<table>
		<tr> <td><b> Samples  </b></td> <td><input id=Samples  type=number min=1 value=512          onchange="upload();"></td></tr>
		<tr> <td><b> TrigLev  </b></td> <td><input id=TrigLev  type=number min=1 value=511 max=1023 onchange="upload();"></td></tr>
		<tr> <td><b> TimeBase </b></td> <td><input id=TimeBase type=number min=1 value=100          onchange="upload();"></td></tr>
	</table>
</div>

</body>
</html>
`
