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

function HideClass(classToHide) {
	var x = document.getElementsByClassName(classToHide);
	var i;
	for (i = 0; i < x.length; i++) {
	  x[i].style.display='none';
	}
}

function ShowClass(classToShow) {
	var x = document.getElementsByClassName(classToShow);
	var i;
	for (i = 0; i < x.length; i++) {
	  x[i].style.display='';
	}
}

function HideId(IdToHide) {
    document.getElementById(IdToHide).display='none';
}

function ShowId(IdToShow) {
    document.getElementById(IdToShow).display='';
}