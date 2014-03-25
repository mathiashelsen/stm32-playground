package main

// Serves the scope's main page.

import(
	"fmt"
	"net/http"
)

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

	</script>

</head>

<body>
	
	<h1>Poor man's scope</h1>

	<img id="screen" src="/screen.svg" />

</body>
</html>
`
