package main

// Serves the scope's main page.

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
)

func rootHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintln(w, page)
}

func eventHandler(w http.ResponseWriter, r *http.Request) {

	switch r.Method {
	//	case "GET":
	//		json.NewEncoder(w).Encode(&state)

	case "PUT":

		m := make(map[string]interface{})
		check(json.NewDecoder(r.Body).Decode(&m))
		fmt.Println("PUT:", m)
		state := Header{
			Samples: atoi(m["Samples"]), 
			TrigLev: atoi(m["TrigLev"])}
		fmt.Println("WriteToTTY:", state)
		state.WriteTo(serial)
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

var tick = 200;

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


// onreadystatechange function for update http request.
// updates the DOM with new values received from server.
function updateDOM(req){
	if (req.readyState == 4) { // DONE
		if (req.status == 200) {	
			var resp = JSON.parse(req.responseText);	
			setAttr("Samples", "value", resp["Samples"]);
			setAttr("TimeBase", "value", resp["TimeBase"]);
			setAttr("TrigLev", "value", resp["TrigLev"]);
			setAttr("SoftGain", "value", resp["SoftGain"]);
		} 
	}
}

function refresh(){
	setAttr("screen", "src", "/screen.svg?" + Math.random()) // refresh screen.svg, random cachebreaker

	//var req = new XMLHttpRequest();
	//req.open("GET", document.URL + "/event", true); 
	//req.timeout = 2*tick;
	//req.onreadystatechange = function(){ updateDOM(req) };
	//req.setRequestHeader("Content-type","application/x-www-form-urlencoded");
	//req.send("");
}

setInterval(refresh, tick);

function val(id){
	return elementById(id).value
}

function upload(){
	var req = new XMLHttpRequest();
	req.open("PUT", document.URL + "/event", false);
	var map = {
		"Samples": val("Samples"),
		"TrigLev": val("TrigLev"),
		"TimeBase": val("TimeBase"),
		"SoftGain": val("SoftGain")};
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
		<tr> <td><b> Samples  </b></td> <td> <input id=Samples  type=number min=1 value=512           onchange="upload();"></td></tr>
		<tr> <td><b> TrigLev  </b></td> <td> <input id=TrigLev  type=number min=0 value=2000 max=4096 onchange="upload();"></td></tr>
		<tr> <td><b> TimeBase </b></td> <td> <input id=TimeBase type=number min=1 value=100           onchange="upload();"></td></tr>
		<tr> <td><b> SoftGain </b></td> <td>-<input id=SoftGain type=number min=0 value=2             onchange="upload();"></td></tr>
	</table>
</div>

</body>
</html>
`
