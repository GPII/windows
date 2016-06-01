cd c:\vagrant
npm install

mkdir c:\node_modules
cd c:\node_modules
git clone --depth 1 https://github.com/GPII/universal.git
cd universal
npm install
npm install dedupe-infusion
node -e "require('dedupe-infusion')()"
