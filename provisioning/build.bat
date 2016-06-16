cd c:\vagrant
call npm install

mkdir c:\node_modules
cd c:\node_modules
git clone --depth 1 https://github.com/GPII/universal.git
cd universal
call npm install
call npm install dedupe-infusion
node -e "require('dedupe-infusion')()"

