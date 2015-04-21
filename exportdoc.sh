#!/bin/sh
if [ "$TRAVIS_REPO_SLUG" = "Lectem/ctrulib" ] && [ "$TRAVIS_PULL_REQUEST" = "false" ] && [ "$TRAVIS_BRANCH" = "master" ]; then
sudo apt-get install -qq doxygen
git clone --branch=gh-pages --single-branch --depth 1 https://${GH_TOKEN}@github.com/Lectem/ctrulib docs
cd docs
git rm -rf ./*
cd ..
doxygen libctru/Doxyfile
cd docs
mv ./html/* .
git add --all
git commit -m"Doc generated from Travis build #$TRAVIS_BUILD_NUMBER"
git push -f origin gh-pages

fi