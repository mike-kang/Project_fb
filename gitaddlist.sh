#!/bin/sh

git status | grep modified | awk '{print $3}'

