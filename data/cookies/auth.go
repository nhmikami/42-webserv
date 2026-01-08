package main

import (
    "fmt"
    "os"
    "strings"
    "time"
)

func main() {
    // Ler cookies existentes
    cookies := make(map[string]string)
    if cookieHeader := os.Getenv("HTTP_COOKIE"); cookieHeader != "" {
        cookiePairs := strings.Split(cookieHeader, "; ")
        for _, pair := range cookiePairs {
            parts := strings.SplitN(pair, "=", 2)
            if len(parts) == 2 {
                cookies[parts[0]] = parts[1]
            }
        }
    }

    // Calcular data de expiração (1 hora)
    expires := time.Now().Add(1 * time.Hour).UTC().Format(time.RFC1123)

    // Enviar headers
    fmt.Println("Content-Type: text/html")
    fmt.Println("Set-Cookie: user_id=123; Path=/; Max-Age=3600")
    fmt.Printf("Set-Cookie: session_token=abc123xyz; Path=/; Expires=%s; HttpOnly\r\n", expires)
    fmt.Println("Set-Cookie: theme=dark; Path=/; Secure; SameSite=Strict")
    fmt.Println() // Linha vazia para separar headers do body

    // Enviar HTML
    fmt.Println("<html>")
    fmt.Println("<head><title>Teste de Cookies - Go CGI</title></head>")
    fmt.Println("<body>")
    fmt.Println("<h1>🍪 Teste de Cookies em Go</h1>")
    
    fmt.Println("<h2>Cookies Recebidos:</h2>")
    if len(cookies) > 0 {
        fmt.Println("<ul>")
        for key, value := range cookies {
            fmt.Printf("<li><strong>%s</strong>: %s</li>\n", key, value)
        }
        fmt.Println("</ul>")
    } else {
        fmt.Println("<p><em>Nenhum cookie recebido</em></p>")
    }

    fmt.Println("<h2>Cookies Enviados:</h2>")
    fmt.Println("<ul>")
    fmt.Println("<li><strong>user_id</strong>=123 (expira em 1 hora)</li>")
    fmt.Printf("<li><strong>session_token</strong>=abc123xyz (expira em: %s, HttpOnly)</li>\n", expires)
    fmt.Println("<li><strong>theme</strong>=dark (Secure, SameSite=Strict)</li>")
    fmt.Println("</ul>")

    fmt.Println("<h2>Informações do Ambiente:</h2>")
    fmt.Println("<ul>")
    fmt.Printf("<li><strong>REQUEST_METHOD</strong>: %s</li>\n", os.Getenv("REQUEST_METHOD"))
    fmt.Printf("<li><strong>QUERY_STRING</strong>: %s</li>\n", os.Getenv("QUERY_STRING"))
    fmt.Printf("<li><strong>REMOTE_ADDR</strong>: %s</li>\n", os.Getenv("REMOTE_ADDR"))
    fmt.Println("</ul>")

    fmt.Println("<hr>")
    fmt.Println("<p><a href='/authentication/auth.go'>Recarregar página</a></p>")
    fmt.Println("</body>")
    fmt.Println("</html>")
}