$fn=20;

length = 160;
width= 120;
height = 40;
cornerRadius = 10;
//joystick screws
joystick_Screw = 2;  //usually 2mm
joystick_Screw_CS = 3.5; //countersink area for screw
joystick_Screw_CS_H = 0.8; //countersitck depth
joystick_Holes_X = 32.5; //32.5 for FTF-D300 joystick
joystick_Holes_Y = 32.5; //32.5 for FTF-D300 joystick;

//translate([29,80,9]) rotate([180,0,00]) import("Mega_2560_PRO_Embed_CH340G.stl");


//Some print
//translate([(width/2),65,-10]) rotate([180,0,180]) linear_extrude(4) text("Fauteuil spatial");

difference(){
    translate([10, 10, 0]){
        difference() {
            roundedBox(length, width, height, cornerRadius); 
            translate([1,1,1]) {
                roundedBox(length-2, width-2, height-1, cornerRadius); 
            }
            //Joystick support
            translate([-64, -41,0]) joystick_support(0.8);
            translate([3.5, -41,0]) joystick_support(0.8);  
        }
    }
    //Odysseus Logo
translate([(width/2),65,-0.8]) rotate([180,0,180]) scale([0.25, 0.25, 0.01]) surface(file = "logo_fond-sombre-02.png", center = true, invert = true);
    //Screen assembly
translate([(width/2),130,-9]) rotate([-90,0,90]) import("TFT_housing.stl");
    //Hole for cable screen
    translate([22,111,0]) cube([4,38,2]);
    //Arduino USB hole
    translate([0,69,4]) cube([2,11,4]);
    //Hole for SP13 plug real diameter is 13, and real recession is 12.2
    //will add 1.25mm to make it ok	
	translate ([60,160,15]) rotate([90,0,0]) intersection(){

            tolerance=1.25;
            cylinder(6,d=(13+tolerance), center=true, $fn=50);
            translate([0.4,0,0]) cube([(12.2+tolerance),(13+tolerance),6], center=true);
        }
}
//Only availble for settings position
//translate([58,49,0]) rotate([0,0,180]) joystick();
//translate([73,59,0]) rotate([0,0,-90]) joystick();





translate([width*2, 10, 0]){
    mirror([1,0,0]) {
        roundedBox(length, width, 1, cornerRadius);
        difference() {
            translate([1,1,0]) {
                roundedBox(length-2,width-2,4,cornerRadius);
            }
            translate([2,2,0]) {
                roundedBox(length-4,width-4,4,cornerRadius);
            }
        }
    }
}

/*
//handle
translate([0,length,0]) {
    difference() {
        difference() {
            cylinder(h=2, r=10);
            cylinder(h=2, r=5);

        }
        translate([0,-10,0]) {
            cube(size=[10,10,2]);        
        }
    }
}
*/
translate([15.5,69.5,0]) hole();
translate([53.3,76.3,0]) hole();

module hole(){
    difference(){
    cylinder(h=7.4,d=4);
    cylinder(h=7.4,d=2);
    }
}

module roundedBox(length, width, height, radius)
{
    dRadius = 2*radius;

/*
    //cube bottom right
    translate([width-dRadius,-radius,0]) {
        cube(size=[radius,radius,height+0.01]);
    }

    //cube top left
    translate([-radius,length-dRadius,0]) {
        cube(size=[radius,radius,height+0.01]);
    }
*/
    //base rounded shape
    minkowski() {
        cube(size=[width-dRadius,length-dRadius, height]);
        cylinder(r=radius, h=0.01);
    }
  
    
  
}

module joystick(){
    union(){
        translate ([10,0,0]) cube([40,40,25]);
        translate([0,5,0]) cube([10,30,25]);
        translate([15,-10,0]) cube([30,10,25]);
    }
}

module joystick_support(height){
            //Screws for joystick
        translate([(length/2)+(joystick_Holes_X/2),(width/2)+(joystick_Holes_Y/2),0]) cylinder(r=joystick_Screw/2, h=height, $fn=40);
        translate([(length/2)+(joystick_Holes_X/2),(width/2)-(joystick_Holes_Y/2),0]) cylinder(r=joystick_Screw/2, h=height, $fn=40);
        translate([(length/2)-(joystick_Holes_X/2),(width/2)+(joystick_Holes_Y/2),0]) cylinder(r=joystick_Screw/2, h=height, $fn=40);
        translate([(length/2)-(joystick_Holes_X/2),(width/2)-(joystick_Holes_Y/2),0]) cylinder(r=joystick_Screw/2, h=height, $fn=40);
        
        //Countersink for joystick screws
        translate([(length/2)+(joystick_Holes_X/2),(width/2)+(joystick_Holes_Y/2),0]) rotate_extrude($fn=40) polygon( points=[[0,0],[joystick_Screw_CS/2,0],[1,joystick_Screw_CS_H],[0,joystick_Screw_CS_H]] );
        translate([(length/2)+(joystick_Holes_X/2),(width/2)-(joystick_Holes_Y/2),0]) rotate_extrude($fn=40) polygon( points=[[0,0],[joystick_Screw_CS/2,0],[1,joystick_Screw_CS_H],[0,joystick_Screw_CS_H]] );
        translate([(length/2)-(joystick_Holes_X/2),(width/2)+(joystick_Holes_Y/2),0]) rotate_extrude($fn=40) polygon( points=[[0,0],[joystick_Screw_CS/2,0],[1,joystick_Screw_CS_H],[0,joystick_Screw_CS_H]] );
        translate([(length/2)-(joystick_Holes_X/2),(width/2)-(joystick_Holes_Y/2),0]) rotate_extrude($fn=40) polygon( points=[[0,0],[joystick_Screw_CS/2,0],[1,joystick_Screw_CS_H],[0,joystick_Screw_CS_H]] );
        
        
        //Opening hole for joystick
        //Should be d=36, but imprecision make me add 0.5mm
        translate([length/2, width/2, 0]) cylinder(r=18.5, h=height, $fn=50);
}