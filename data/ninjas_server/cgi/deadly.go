package main

import (
	"fmt"
)

func main() {
	fmt.Println("Content-Type: text/html")
    fmt.Println()
	
	html:= `<!DOCTYPE html>
	<html>
		<head>
			<title>Deadly Mode</title>
			<link rel="stylesheet" href="/style.css">
		</head>
	
		<body class="bg-deadly deadly-container">
			<p class="title">Kill Enemy</p>
	
			<form method="POST" action="/deadly/deadly.go">
				<button class="btn" type="submit">Click to kill enemy</button>
			</form>
		</body>
	</html>
	`
	
	fmt.Print(html)
	
}
