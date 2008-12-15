ViewRedirect { // Abstract class
	*implClass { ^GUI.scheme.perform(this.key) }
	*new { arg parent, bounds; ^this.implClass.new(parent, bounds) }
	*browse { ^this.implClass.browse }
	*doesNotUnderstand{|selector ... args|	^this.implClass.perform(selector, *args)  }
}


Window : ViewRedirect { 
	*key { ^\window }
	*new { arg name = "panel", bounds, resizable = true, border = true, server, scroll = false;
		^this.implClass.new(name, bounds, resizable, border, server, scroll)
	}
}

CompositeView : ViewRedirect { *key { ^\compositeView }}
ScrollView : ViewRedirect { *key { ^\scrollView }}
HLayoutView : ViewRedirect { *key { ^\hLayoutView }}
VLayoutView : ViewRedirect { *key { ^\vLayoutView }}

Slider : ViewRedirect { *key { ^\slider }}

//Knob : SCSlider {
//}

//Font : ViewRedirect { *key { ^\font }}
Pen : ViewRedirect { *key { ^\pen }}

Stethoscope : ViewRedirect { *key { ^\stethoscope }}
ScopeView : ViewRedirect { *key { ^\scopeView }}
FreqScopeView : ViewRedirect { *key { ^\freqScopeView }} // redirects to SCFreqScope
FreqScope : ViewRedirect { *key { ^\freqScope }} // redirects to SCFreqScopeWindow
Dialog : ViewRedirect { *key { ^\dialog }}
View : ViewRedirect { *key { ^\view }}

RangeSlider : ViewRedirect { *key { ^\rangeSlider }}
Slider2D : ViewRedirect { *key { ^\slider2D }}
TabletSlider2D : ViewRedirect { *key { ^\tabletSlider2D }}
Button : ViewRedirect { *key { ^\button }}

PopUpMenu : ViewRedirect { *key { ^\popUpMenu }}
StaticText : ViewRedirect { *key { ^\staticText }}
NumberBox : ViewRedirect { *key { ^\numberBox }}
ListView : ViewRedirect { *key { ^\listView }}

DragSource : ViewRedirect { *key { ^\dragSource }}
DragSink : ViewRedirect { *key { ^\dragSink }}
DragBoth : ViewRedirect { *key { ^\dragBoth }}

UserView : ViewRedirect { *key { ^\userView }}
MultiSliderView : ViewRedirect { *key { ^\multiSliderView }}
EnvelopeView : ViewRedirect { *key { ^\envelopeView }}

TextField : ViewRedirect  { *key { ^\textField }}


TabletView : ViewRedirect { *key { ^\tabletView }}
SoundFileView : ViewRedirect { *key { ^\soundFileView }}
MovieView : ViewRedirect { *key { ^\movieView }}
TextView : ViewRedirect  {	*key { ^\textView }}