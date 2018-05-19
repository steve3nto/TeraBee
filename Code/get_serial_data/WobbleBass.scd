
NetAddr("10.79.9.169",57121);
thisProcess.openUDPPort(57121);


// ------ Wobble bass complete -----

(
SynthDef(\dubstep, { arg freq = 22,gate = 1, dur=2;
	var trig, son, sweep, out;

	//freq = note.midicps;

	trig = CoinGate.kr(0.5, Impulse.kr(2));
	//trig = Impulse.kr(2);

	sweep = Poll.kr(trig,LFSaw.ar(Demand.kr(trig, 0, Drand([1, 2, 2, 3, 4, 5, 6, 8, 16], inf))).exprange(40, 5000));   /// Inf: number of different values to produce

	son = Mix.ar(LFSaw.ar(freq * [0.99, 1, 1.01]));
    son = LPF.ar(son, sweep);
    out = son + BPF.ar(son, 2000, 5);

	a = EnvGen.ar(Env.asr(0.01,1,dur),gate:gate,doneAction:2);

	Out.ar(0, Pan2.ar(FreeVerb.ar(out*a,room:0.3),0));
}).add;
)


x = Synth(\dubstep,[\freq,300]);
x.set(\freq,200);
x.set(\freq,1000);
x.set(\gate,0);


(
var synth1;
synth1 = Synth(\dubstep,\t_trig,1);

OSCdef('OSCreceiver',
	{
		arg msg;

		var freq = midicps(msg[1]);
		//var cutoff = 100*(msg[4]+msg[5]+20);

		synth1.set(\freq,freq,\t_trig,1);
		msg.postln();

	},
	"/SC/inputs");

)