
	
		var skin = GUI.skin;
		
				if(proxy.notNil) { proxy.vol_(ampSpec.map(slid.value)); } 
			});
			;
			// swingosc: 16 is normal  24 is alt, ctl is off, and fn is 16 as well
				.font_(font)
					["-<", skin.fontColor, skin.onColor]
				.action_({ |box, mod|
					box.value_(1 - box.value)
			.font_(font)
			.action_({ arg btn; 
			.font_(font)
				mod.postln;
					// alt-click osx, swingosc
				btn.value_(1 - btn.value)
			;
		 
		var currVol=0, pxname="", isAudio=false, plays=0, playsSpread=false, pauses=0, canSend=0; 
		
		
		
			canSend = proxy.objects.notEmpty.binaryValue;
			
			isAudio = proxy.rate == \audio;
			plays = monitor.isPlaying.binaryValue;
			
			if (monitor.notNil, { 
				playsSpread = proxy.monitor.hasSeriesOuts.not;
		};
		
		currState = [currVol, pxname, isAudio, plays, outs, playsSpread, pauses, canSend];
		
		if (currState != oldState) { 
		//	"updating".postln; 
		
			ampSl.value_(currVol);
			nameBut.object_(pxname);
			pauseBut.value_(pauses);
			playBut.value_(plays);
			sendBut.value_(canSend);

			if (isAudio != oldState[2]) { 
				ampSl.enabled_(isAudio);
				playBut.enabled_(isAudio);
				setOutBox.enabled_(isAudio);
				playNDialogBut.enabled_(isAudio);
			}; 
			
			if (setOutBox.hasFocus.not) {	 
				setOutBox.value_(try { outs[0] } ? 0);
				if (usesPlayN) { 
					playNDialogBut.value_(playsSpread.binaryValue)
				};
			}
		};
		oldState = currState;