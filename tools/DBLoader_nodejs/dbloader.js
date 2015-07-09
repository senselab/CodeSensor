
var GoogleCryptoJS = require ("./hmac-sha1.js")
var util = require('util');
var fs = require('fs');
var parseXlsx = require('excel');
var printf = require('printf');
var assert = require('assert');
var async = require('async');
var mysql = require('mysql');	
var student_data;
var mysql_account = 'codesensor';
var mysql_pwd = 'nctucodesensor';
var dbname = 'code_sensor_2015';


async.waterfall(

	[
		function(callback) {
			/* for loading excel format studnet roster
			parseXlsx('student_roster.xlsx', function(err,data) {
				if (err) 
					throw err;
				//printTwoDimArray(data);
				//console.log("World");
				student_data = ExtractStudentData(data);
				callback(null);
			});*/
			parseTextRoster('student_roster.txt', function(data) {
				student_data = data;
				callback(null);
			});
		},

		function(callback) {
			printf(process.stdout, "\nA total of %d students loaded\n\n", student_data.length );

			//console.log(mysql);
			var conn = mysql.createConnection({
											host:'localhost',
											user: mysql_account,
											password: mysql_pwd
										})	;

			conn.connect();

			conn.query('USE '+dbname, function(err){
				if (err) {
					console.log('failed to use database ' + dbname);
					process.exit(-1);
				}
			});

			var pending_queries = 0;

			student_data.forEach(function(student) {
				//printf(process.stdout, "%s %s\n", student.ID, student.NAME);

				var id = student.ID;
				var name = student.NAME;
				var email = "";
				var dept = "";
				var pwd_plain = "algo"+id;

				var pwd_hash = GoogleCryptoJS.CryptoJS.HmacSHA1(pwd_plain, 'sense lab');			
				var pwd = pwd_hash.toString(GoogleCryptoJS.CryptoJS.enc.Base64);
				

				var delete_stmt = util.format("DELETE FROM `student_roster` where `id`='%s'", id);

				console.log(delete_stmt)	;

				pending_queries++;	

				conn.query(delete_stmt, function(err, rows, fields) {
					if (err) throw err;

					var insert_stmt = util.format ("INSERT INTO `student_roster` (`id`,`name`,`email`,`dept`,`pwd`) VALUES ('%s','%s','%s','%s','%s')",id, name, email, dept, pwd);

					console.log(insert_stmt);

					conn.query(insert_stmt, function(err, rows, fields) {
						if (err) throw err;

						pending_queries--;

						if (pending_queries ==0) {
							conn.end();
							console.log("\n\nDone !\n");
							callback(null);
						}						
					});

				});

			});

			
/*
			conn.query('select * from student_roster', function(err, rows, fields) {
				if (err) throw err;
				console.log(rows);
				//console.log(fields);
			});
*/
						
		}
	],

	function(err, caption) {
		//console.log(caption);
	}

);


//console.log("Data loaded successfully.");

//printf(process.stdout, "Hi" );
//console.log(student_data);
//printf(process.stdout, "\nA total of %d students loaded\n", student_data.length );

/////////////////////////////////////////////////////////////////


function printTwoDimArray(array)
{
	var k,m;

	for ( k = 0; k < array.length; k++) {
		for ( m = 0; m < array[k].length; m++) {
			printf(process.stdout, "[%s]\t", array[k][m])	;
		}

		printf(process.stdout, "\n");
	}

}

function trim_space(s)
{
	s = s.replace(/(^\s*)|(\s*$)/gi, "");
	return s;
}

function parseTextRoster(filename, callback)
{
	var student_data = new Array();

	var lines = fs.readFileSync(filename).toString().split("\n");
	
	for ( i in lines ) {
		elements = lines[i].split(/[\s,]+/);

		if (elements.length !=2) continue;
//		console.log(elements);
		student_data.push({ID:elements[0], NAME:elements[1]});
	}


	callback(student_data);
}

function ExtractStudentData(array)
{
	var student_data = new Array();
	var k,m;
	var regex_id = new RegExp(/^\s*[\dA-Z]\d{6}\s*$/);
//	var regex_id = new RegExp(/\d+/);
	//var cnt = 0;

	for ( k = 0; k < array.length; k++) {
		var id = null;
		var name = null;


		for ( m = 0; m < array[k].length; m++) {

			if ( !id ) {

				var id_t = regex_id.exec(array[k][m]);
				if ( id_t) {
				//	printf(process.stdout, "%s\t", id[0]);
					id = id_t[0];
				}
			}
			else if ( !name) {
				name = 	trim_space(array[k][m]);	
			}
			else
				break;			
		}

		if ( id) {

			assert(name, "badly formated xls");

			student_data.push( {ID:id, NAME:name})	;

		//	printf(process.stdout, "[%s] [%s]\n", id, name);
		//	cnt++;
		}
	}

	//printf(process.stdout, 'A total of %d students loaded\n', cnt);
	return student_data;

}

