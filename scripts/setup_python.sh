cd $(cd -P -- "$(dirname -- "$0")" && pwd -P)
python3 -m virtualenv venv
source venv/bin/activate
pip install pyowm
