
docker build -f HOST/custom/uppercase/containerization/docker/Dockerfile --tag cfp_uppercase:1.0 .

docker run -it --rm -p 8888:8888 --network="host" cfp_uppercase:1.0
