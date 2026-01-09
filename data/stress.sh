# Requisições sequenciais:
for i in {1..1000}; do
  curl -s $URL > /dev/null
done

# Requisições Simultâneas
for i in {1..100}; do
  curl -s $URL > /dev/null &
done
wait

# Explodir número de conexões
for i in {1..500}; do
  curl -s $URL > /dev/null &
done
wait

# Cliente lento
curl --limit-rate 1B $URL

# Conexão que demora para enviar dados
curl --limit-rate 10B -X POST -d "aaaaaaaaaaaaaaaaaaaaaaaaaaaa" $URL

# Timeout agressivo
curl --connect-timeout 1 --max-time 2 $URL

# Keep Alive
curl --keepalive-time 60 --http1.1 -v $URL

# Headers enormes (stress de parsing)
curl -H "X-Test: $(printf 'A%.0s' {1..8000})" $URL


# Métodos não implementados
curl -X PUT $URL
curl -X PATCH $URL
