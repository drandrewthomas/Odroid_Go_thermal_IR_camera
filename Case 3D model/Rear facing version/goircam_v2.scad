$fn=30;
trad=10;
brad=3;

//rotate([0,180,0]) makeheaderadapter();
//rotate([-90,0,0]) makeholddown();
//rotate([-90,0,0]) makefascia();
rotate([0,180,0]) makecover();

module makecover()
{
  color("#0000ff")
  {
    difference()
    {
      translate([0,0,0]) roundedbox(42+5,33+5,18.75+2.5,10+2.5,3);
      translate([0,0,0]) roundedbox(42+1,33+1,18.75+2.5-3,10,3);
      translate([0,10,0]) roundedbox(42-5+2,33-5+2,5,8,3);
      translate([0,0,-33+10.5-2]) cube([100,100,33],center=true);
      intersection()
      {
        translate([0,10,2.25]) roundedbox(42.5,33,18.75,10,3);
        translate([0,10,-16+2.25]) cube([100,100,16],center=true);
      }
    }
  }
}

module makeheaderadapter()
{
  difference()
  {
    roundedbox(42,33,18.75,10,3);
    translate([0,0,-16]) cube([45,14.7,16],center=true);
    translate([0,3.4,-16.5+8]) cube([26,3,6],center=true);
    translate([0,0,33-25+2.5]) cube([45,45,25+8],center=true);
    translate([21-1.5,-1,-6]) cube([3+1,8+1,40],center=true);
    translate([-21+1.5,-1,-6]) cube([3+1,8+1,40],center=true);
  }
}

module makeholddown()
{
  color("#ff0000")
  {
    union()
    {
      difference()
      {
        roundedbox(42,33,18.75,10,3);
        translate([0,0,-33+10.5+0.2]) cube([100,100,33],center=true);
        translate([0,18.75/2,0]) cube([100,3,100],center=true);
        translate([21,-3.5,12.5+3]) cube([56,18.75,33],center=true);
        translate([21-1,-3.5,12.5+3+10]) cube([56,18.75,33],center=true);
        translate([-21,-3.5,12.5+3]) cube([16,18.75,33],center=true);
        translate([0,-5-9.5,12.5+3]) cube([50,18.75,33],center=true);
        translate([0,0,-4.35]) cube([29,50,3],center=true);
        translate([4,-18.75/2,-2]) cube([21,16,3],center=true);
        translate([-4,-18.75/2,-2]) cube([21,8.5,3],center=true);
        translate([-7,8,6.5]) rotate([90,0,0]) cylinder(h=50,r=5,center=true);
        translate([-10,-5,6.5+(13.75/2)]) rotate([90,0,0]) cylinder(h=15,r=1,center=true);
        translate([-10,-5,6.5-(13.75/2)]) rotate([90,0,0]) cylinder(h=15,r=1,center=true);
      }
      translate([21-1.5,-1,-6]) cube([3,8,4],center=true);
      translate([-21+1.5,-1,-6]) cube([3,8,4],center=true);
    }
  }
}

module makefascia()
{
  color("#00ff00")
  {
    difference()
    {
      roundedbox(42-5+2-0.5,33-5+2-0.5,1,8,3);
      translate([0,0,-33+10.5+0.2+0.25]) cube([100,100,33],center=true);
      translate([-7,8,6.5]) rotate([90,0,0]) cylinder(h=50,r=7,center=true);
      difference()
      {
        translate([10.5,8,6.5]) rotate([90,0,0]) cylinder(h=50,r=6,center=true);
        translate([10.5,8,6.5]) cube([3,100,100],center=true);
        translate([10.5,8,6.5]) cube([100,100,3],center=true);
        translate([10.5,8,6.5]) rotate([90,0,0]) cylinder(h=50,r=3.5,center=true);
      }
      translate([10.5,8,6.5]) rotate([90,0,0]) cylinder(h=50,r=2,center=true);
    }
  }
}

module roundedbox(w,h,d,trad,brad)
{
  hull()
  {
    translate([-(w/2)+brad,0,-(h/2)+brad]) rotate([90,0,0]) cylinder(h=d,r=brad,center=true);
    translate([(w/2)-brad,0,-(h/2)+brad]) rotate([90,0,0]) cylinder(h=d,r=brad, center=true);
    translate([(w/2)-trad,0,(h/2)-trad]) rotate([90,0,0]) cylinder(h=d,r=trad,center=true);
    translate([-(w/2)+trad,0,(h/2)-trad]) rotate([90,0,0]) cylinder(h=d,r=trad,center=true);
  }
}
