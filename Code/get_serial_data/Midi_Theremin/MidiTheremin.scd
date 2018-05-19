
NetAddr("127.0.0.1",57121);
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
SynthDef(\risset, { arg out= 0, pan= 0, freq= 400, amp= 0.1, dur= 2, t_trig=1;
	var amps= [1, 0.67, 1, 1.8, 2.67, 1.67, 1.46, 1.33, 1.33, 1, 1.33];
	var durs= [1, 0.9, 0.65, 0.55, 0.325, 0.35, 0.25, 0.2, 0.15, 0.1, 0.075];
	var frqs= [0.56, 0.56, 0.92, 0.92, 1.19, 1.7, 2, 2.74, 3, 3.76, 4.07];
	var dets= [0, 1, 0, 1.7, 0, 0, 0, 0, 0, 0, 0];    // Detune
	var src= Mix.fill(11, {|i|
		var env= EnvGen.ar(Env.perc(0.005, dur*durs[i], amps[i], -4.5),t_trig);
		SinOsc.ar(freq*frqs[i]+dets[i], 0, amp*env);
	});
	Out.ar(out, Pan2.ar(src, pan));
}).add;
)

(
SynthDef(\moog, {
    arg vibr=0.5, cutoff=0.5, freq=440, amp=0;
    var osc1, osc2, osc3, f0, vib_int, cutoff_freq, delay, fil_osc;
    vib_int=0.7;

    f0=exp(vibr*(0.05*vib_int))*freq;

    osc1=Saw.ar(f0*1.0013);
    osc2=Saw.ar(f0*0.998);
    osc3=Saw.ar(f0*0.5);


    cutoff_freq=((cutoff+0.5)*6)*freq;

    fil_osc=BLowPass.ar(in:osc1+osc2+osc3, freq:cutoff_freq);

    delay=DelayN.ar(in:fil_osc, delaytime:0.5)!2;

    Out.ar(0, amp*delay);}).add;
)

/*(
Routine({
	a= Synth(\risset);
	20.do{
		var dur= 0.2.exprand(3.0);
		var fre= 60.0.exprand(5000.0);
		("dur:"+dur+"fre:"+fre).postln;
		a.set(\freq, fre, \dur, dur, \t_trig,1);
		dur.wait;
	};
	a.free;
	"done".postln;
}).play;
)*/


(
var wobble;
var moog, moog2, moog3;
wobble = Synth(\dubstep,\t_trig,1);
moog = Synth(\moog);
moog2 = Synth(\moog);
moog3 = Synth(\moog);


OSCdef('OSCreceiver',
	{
		arg msg;

		var freq_chord = midicps(msg[5]);
		var freq_hi = midicps(msg[1]);

		moog.set(\freq, freq_chord);
		moog.set(\amp, 0.5);


		moog2.set(\freq, midicps(msg[5]+4));
		moog2.set(\amp, 0.5);


		moog3.set(\freq, midicps(msg[5]+7));
		moog3.set(\amp, 0.5);


		wobble.set(\freq,freq_hi,\t_trig,1);
		msg.postln();

	},
	"/SC/inputs");

)