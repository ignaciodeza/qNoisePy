# syntax=docker/dockerfile:1
FROM python:3.11-slim-bookworm

WORKDIR /app
RUN apt-get update && apt-get install -y g++
COPY requirements.txt requirements.txt 
RUN pip install -r requirements.txt
COPY . /app
RUN g++ -O3 -Wall -shared -std=c++11 -fPIC $(python3 -m pybind11 --includes) qNoisePy.cpp -o qNoisePy$(python3-config --extension-suffix)


CMD [ "python", "-m" , "qnoise_api", "run", "--host=0.0.0.0"]