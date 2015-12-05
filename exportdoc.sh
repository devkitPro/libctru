#!/bin/sh
if [ "$TRAVIS_REPO_SLUG" = "smealum/ctrulib" ] && [ "$TRAVIS_PULL_REQUEST" = "false" ] && [ -n "$TRAVIS_TAG" ]; then
git clone --branch=gh-pages --single-branch --depth 1 https://${GH_TOKEN}@github.com/$TRAVIS_REPO_SLUG docs
git rm -rf docs/*
cd libctru
doxygen Doxyfile
mv ./docs/html/* ../docs
cd ../docs
git add --all
git commit -m"Doc generated from commit $TRAVIS_COMMIT"
git push -f origin gh-pages

fi
