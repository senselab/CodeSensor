
var GoogleCryptoJS = require ("./hmac-sha1.js")
var util = require('util');
var printf = require('printf');
var assert = require('assert');
var read = require('read');

var stdin = process.stdin;
var stdout = process.stdout;


read({ prompt: 'Plain Password : ', silent: true }, function(er, password) {

	var pwd_plain = password;
    var pwd_hash = GoogleCryptoJS.CryptoJS.HmacSHA1(pwd_plain, 'sense lab');
    var pwd = pwd_hash.toString(GoogleCryptoJS.CryptoJS.enc.Base64);


	console.log('Your password is : %s', pwd)
})
/*
stdin.resume();
stdin.setEncoding('utf8');
stdin.once('data',function(chunk) {
	var pwd_plain = chunk.toString().trim();
    var pwd_hash = GoogleCryptoJS.CryptoJS.HmacSHA1(pwd_plain, 'sense lab');
    var pwd = pwd_hash.toString(GoogleCryptoJS.CryptoJS.enc.Base64);

	process.stdout.write('pwd : ');	
	process.stdout.write( pwd);
	process.stdout.write('\n\n');

	process.exit(0);
});

*/
