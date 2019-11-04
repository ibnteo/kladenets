$fn=12;
hole_rad = 1.4;
thumb = 1/2; // 0, 1/2, 1

include <lib/structor.scad>;

module button() {
	translate([14/2,14/2,11]) {
		color([0.4,0.4,0.4]) translate([0,0,7]) import("lib/switch_mx.stl");
		color([0,0.9,0]) import("lib/G20_row_3key.stl");
	}
}

module hole() {
	linear_extrude(5.2)
		offset(r=0.5)offset(r=-0.5)
			square([14,14]);
	/*translate([4,-1,0]) linear_extrude(3.5+0.1)
		offset(r=0.5)offset(r=-0.5)
			square([6,16]);*/
}

color() {
	for (y=[0:2]) {
		for (x=[1:4]) {
			translate([x*20+(y==0 && x<3?-thumb*20:0)+(y==0 && x>2?thumb*20:0)+10.5,y*20+10.5,-0.1]) button();
		}
	}
}

color([1,1,1]) {
	translate([5,5,5])
	rotate([0,90,-90])
	structor3d([1,12]);

	translate([25,75,5])
	rotate([0,90,-90])
	structor3d([1,8]);

	translate([0,5,5])
	rotate([0,90,0])
	structor3d([1,6]);

	translate([130,5,5])
	rotate([0,90,0])
	structor3d([1,6]);
	
	translate([0,0,0]) difference() {
		translate([0,0,-10]) structor3d([6*2+1,3*2+1]);
		translate([0,3*20+10,-10-0.1]) cube([25+0.1,5+0.1,5+0.2]);
		translate([5*20+10+0,3*20+10,-10-0.1]) cube([25+0.1,5+0.1,5+0.2]);
	}

	difference() {
		union() {
			structor3d([6*2+1,3*2+1]);
		}
		translate([0,3*20+10,-0.1]) cube([25+0.1,5+0.1,5+0.2]);
		translate([5*20+10+0,3*20+10,-0.1]) cube([25+0.1,5+0.1,5+0.2]);
		for (y=[0:2]) {
			for (x=[1:4]) {
				translate([x*20+(y==0 && x<3?-thumb*20:0)+(y==0 && x>2?+thumb*20:0)+10.5,y*20+10.5,-0.1]) hole();
			}
		}
	}
}

// ProMicro
color([0,0.5,0])
	translate([31,81,-4])
	rotate([180,180,90])
	resize([40,22.5,5])
	import("lib/Arduino_Pro_Micro.stl");
