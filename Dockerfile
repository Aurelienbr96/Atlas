FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    make \
    nlohmann-json3-dev

WORKDIR /app
COPY . .

# Configure + build
RUN cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build

EXPOSE 8080

CMD ["./build/web_server"]
