function toggleClassHide(classToHide, classToShow) {
	var x = document.getElementsByClassName(classToHide);
	var i;
	for (i = 0; i < x.length; i++) {
	  x[i].style.display='none';
	}
	x = document.getElementsByClassName(classToShow);
	for (i = 0; i < x.length; i++) {
	  x[i].style.display='';
	}
}