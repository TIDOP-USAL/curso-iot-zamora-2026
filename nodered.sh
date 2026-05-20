#! /bin/bash

set -e

curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.3/install.sh | bash

\. "$HOME/.nvm/nvm.sh"

nvm install 24

npm install -g node-red

node-red

firefox --new-window http://localhost:1880
