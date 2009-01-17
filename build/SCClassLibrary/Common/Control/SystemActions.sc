

AbstractSystemAction {

	*init {
		this.objects = List.new;
	}
	
	*add { arg object;
		if(this.objects.isNil) { this.init }; // lazy init
	//	if(this.objects.includes(object).not) { // agree on this
			this.objects.add(object)
	//	}
	}
	
	*remove { arg object;
		this.objects.remove(object)
	}

	*objects { ^this.shouldNotImplement(thisMethod) }
	*objects_ { arg obj; ^this.shouldNotImplement(thisMethod) }
}



	classvar <>freeRemote = false;


		if(clearClocks, {
		objects.do({ arg item; item.doOnCmdPeriod;  });
	
		if(freeServers, {
		era = era + 1;
	}
		SystemClock.clear;

	}
		"StartUp done.".postln;
	
	*initClass {
		this.objects = IdentityDictionary.new;
	}

		
		if (server.isNil) {
		
			Server.set.do({ arg s; this.add(object, s) });
		
		}{
		
			if (this.objects.includesKey(server).not) {
				this.objects.put(server, List.new);
			};
		
		//	if ( this.objects.at(server).includes(object).not ) {
				this.objects.at(server).add(object)
		//	};
		
		}
	}
	
	
	
	*remove { arg object, server;
		
		if (server.isNil) {
			Server.set.do({ arg s; this.remove(object, s) });
		}{
			if (this.objects.includesKey(server)) {
				this.objects.at(server).remove(object)
			};
		};
		
	}

	
	*run { arg server;
	}

}
		this.objects = IdentityDictionary.new;
	}


		this.objects.at(server).do({ arg item; item.doOnServerQuit(server)  });
		//postf("% ServerQuit\n", server);
	}
	

ServerTree : ServerBoot {

	classvar <>objects;


	*initClass {
		this.objects = IdentityDictionary.new;
	}

	*run { arg server;
		this.objects.at(server).do({ arg item; item.doOnServerTree(server)  });
		//postf("% ServerTree\n", server);
	}
	
}

		UI.registerForShutdown({ this.run });
	}

	//	"ShutDown done.".postln;
