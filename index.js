const addon = require('./build/Release/addon');

async function test() {
  const t = addon.main(function (res) {
    console.log('Addon Result: ' + res);
  });

  console.log("t", t);
}

test();

console.log('Async work started.')