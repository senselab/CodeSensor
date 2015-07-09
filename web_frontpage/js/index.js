$('document').ready(function(){
	//$('div#header').css('top',-500);
	/*
	$('div#header').animate({
		top: "0"
	}, 1000, 'swing', 
	function(){
		buttonEnable();
	});*/
});

function scrollUp(url){
	$('div#header').animate({
		top: "-=500"
	}, 1000, 'swing', 
	function(){
		window.location.href=url;
	});
}

function buttonEnable(){
	$('button.btn').on('click', function(){
		var url = $(this).attr('href');
		scrollUp(url);
	});
}