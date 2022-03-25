screwnose_Diameter      = 4; // [2:0.2:8]
// Height of the noses
screwnose_Height        = 5; // [2:0.2:10]
// Wall thickness
screwnose_Wall_Thickness = 2.8; // [2:0.2:5]
box_Screw_Corner_Radius   =  6; // [2:1:10]
// four outer screw hole diameters
box_Screw_Diameter     =  3.2; // [2:0.2:4]

module pinch(h=1,r1=1,r2=1,r3=0) {
    zero = 0.00000000001;
    steps = $fn ? $fn : $fa;
    res = h/steps;
    r1 = r1 ? r1 : res;
    r2 = r2 ? r2 : res;
    r3 = r3 ? r3 : zero;
    hull() {
        cylinder(h=zero,r=r1,r2=res,r3=0,center=false);
        translate([-r2, -r3, h - zero]) cube([
            r2 * 2,
            r3 * 2,
            zero
        ]);
    }
}


//Caps to fit on top of joystick
difference (){
rotate_extrude($fn=100) polygon( points=[[18,0],[21,0],[20,17],[19,20],[0,20],[0,17],[16.5,17]] );
translate ([0,0,8.5]) rotate([0,90,0]) cube([17,42,1],true);
};
translate ([0,0,20]) cylinder(h=10, r1=19, r2=6, center=false, $fn=100);
//Hearings caps
translate([1, 19, 5]) rotate([0,90,0]) screwNose(3, 3);
translate([-4, 19, 5]) rotate([0,90,0]) screwNose(3, 3);    
translate([-1, -19, 5]) rotate([180,90,0]) screwNose(3, 3);    
translate([4, -19, 5]) rotate([180,90,0]) screwNose(3, 3); 
translate ([1,18,11]) rotate([-45,0,0]) cube([3,5,3]);
translate ([1,17.9,14]) rotate([-45,0,0]) cube([3,11,2.5]);

translate ([-4,18,11]) rotate([-45,0,0]) cube([3,5,3]);
translate ([-4,17.9,14]) rotate([-45,0,0]) cube([3,11,2.5]);
translate ([1,-22,7.5]) rotate([45,0,0]) cube([3,5,3]);
translate ([1,-25.1,6.7]) rotate([45,0,0]) cube([3,11,2.5]);

translate ([-4,-22,7.5]) rotate([45,0,0]) cube([3,5,3]);
translate ([-4,-25.1,6.7]) rotate([45,0,0]) cube([3,11,2.5]);



//Handle in + shape (rotation)
translate([0, 9, 38]) rotate([0,0,90]) union () {difference () {
union () {
translate([-10,0,0]) rotate([0,-90,0]) cylinder(10,10, 10, center=true, $fn=50);
translate([-15,0,0]) rotate([0,-90,0]) pinch(35, 10, 10, 1, $fn=50);
translate([-5,0,0]) rotate([180,-90,0]) pinch(67, 10, 8, 3.5, $fn=50);
//
};
//Edges
translate ([61,2.7,-9]) difference() { cube([2,2,18]); cylinder(18, 1, 1,  $fn=50);};
translate ([61,-2.7,-9]) rotate ([0,0,-90]) difference() { cube([2,2,18]); cylinder(18, 1, 1,  $fn=50);};
translate ([61,8,7.1]) rotate ([90,0,0]) difference() { cube([2,2,18]); cylinder(18, 1, 1,  $fn=50);};
translate ([61,-8,-7.1]) rotate ([-90,0,0]) difference() { cube([2,2,18]); cylinder(18, 1, 1,  $fn=50);};
translate ([-49,0.8,-10]) rotate ([0,0,90]) difference() { cube([2,2,22]); cylinder(22, 1, 1,  $fn=50);};
translate ([-49,-0.8,-10]) rotate ([0,0,180]) difference() { cube([2,2,22]); cylinder(22, 1, 1,  $fn=50);};
translate ([-49,8,9]) rotate ([90,-90,0]) difference() { cube([2,2,18]); cylinder(18, 1, 1,  $fn=50);};
translate ([-49,-8,-9]) rotate ([-90,90,0]) difference() { cube([2,2,18]); cylinder(18, 1, 1,  $fn=50);};
//Sides
translate ([0,11,0]) cube([80, 5, 32], center=true);
translate ([0,-11,0]) cube([80, 5, 32], center=true);
};
translate ([-10,-8,0]) rotate([90,90,0]) pinch(25, 5.2, 4, 1, $fn=50);
translate ([-10,8,0]) rotate([-90,90,0]) pinch(25, 5.2, 4, 1, $fn=50);
};

translate ([1,-22,7.5]) rotate([45,0,0]) cube([3,5,3]);
translate ([1,-25.1,6.7]) rotate([45,0,0]) cube([3,11,2.5]);

translate ([-4,-22,7.5]) rotate([45,0,0]) cube([3,5,3]);
translate ([-4,-25.1,6.7]) rotate([45,0,0]) cube([3,11,2.5]);






    module screwNose(screwholeDiameter=4, noseHeight=5) {
	additionalDistanceFromWall = 1;
	translate([0,screwholeDiameter/2+screwnose_Wall_Thickness+additionalDistanceFromWall, 0]) difference() {
		union() {
			translate([-(screwholeDiameter/2+screwnose_Wall_Thickness), -(screwholeDiameter/2+screwnose_Wall_Thickness+additionalDistanceFromWall),0]) cube([(screwholeDiameter/2+screwnose_Wall_Thickness)*2, screwholeDiameter/2+screwnose_Wall_Thickness+additionalDistanceFromWall, noseHeight]);
			cylinder(r=(screwholeDiameter/2)+screwnose_Wall_Thickness, h=noseHeight, $fn=60);
		}
		cylinder(r=screwholeDiameter/2, h=noseHeight, $fn=60);
	}
}