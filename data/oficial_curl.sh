#Set variavel de ambiente
URL="http://localhost:8084"

# location /
curl -X GET "$URL"

#location /silent
curl -X GET "$URL/silent"
curl -d "" "$URL/silent/silent.py"

#location /deadly
curl -i -c cookies.txt http://127.0.0.1:8084/deadly/deadly.go # Inicializa o cookie
curl -i -b cookies.txt -c cookies.txt -d "" http://127.0.0.1:8084/deadly/deadly.go
curl -i -b cookies.txt http://127.0.0.1:8084/deadly/deadly.go

#location /forbidden
curl -X GET "$URL/forbidden"

# ================================================== #

#Set variavel de ambiente
URL="http://localhost:8085"

# location /
curl -X GET "$URL"