$fn=12;
structor_base_len = 5.0;

module structor2d(dim, center=false, is_hole_rad=false)
{
	hole_rad = 1.55;
	frez_rad = 1.4;

    x=dim[0];
    y=dim[1];

    hole_to_square= sin(45) * hole_rad;

    tooth_matrix = [[-1,0],[-1,1],[0,1],[0,2]];
    
    left=[
        for (ty=[0:y-1])
            for (tp=[0:len(tooth_matrix)-1])
                [tooth_matrix[tp][0],2*ty+tooth_matrix[tp][1]] 
    ];

    top =[
        for (tx=[0:x-1])
            for (tp=[0:len(tooth_matrix)-1])
                [2*tx+tooth_matrix[tp][1],2*y-1-tooth_matrix[tp][0]] 
    ];   

    right = [
            for (ty=[0:y-1])
                for (tp=[0:len(tooth_matrix)-1])
                    [2*x-1-tooth_matrix[tp][0],2*y-1-2*ty-tooth_matrix[tp][1]] 
    ];

    bottom = [
            for (tx=[0:x-1])
                for (tp=[0:len(tooth_matrix)-1])
                    [2*x-1 - 2*tx-tooth_matrix[tp][1],tooth_matrix[tp][0]] 
    ];   

    lp=[left[0]];
    
    myFig = structor_base_len * concat(left,top,right,bottom,lp);
    
    center_coor =[-x*structor_base_len/2,-y*structor_base_len/2];

    translate(center ? [-structor_base_len*(x-0.5), -structor_base_len*(y-0.5), -structor_base_len/2] : [structor_base_len,structor_base_len])

		difference() {
			polygon(myFig);
	
			union() {
			
				for (sQ_x=[0:x-1]) {
					for (sQ_y = [0:y-1]) {
						translate([(2*sQ_x)*structor_base_len-hole_to_square, (2*sQ_y)*structor_base_len-hole_to_square])
							circle(r=frez_rad);
			
						translate([(2*sQ_x+1)*structor_base_len+hole_to_square, (2*sQ_y)*structor_base_len-hole_to_square])
							circle(r=frez_rad);

						translate([(2*sQ_x+1)*structor_base_len+hole_to_square, (2*sQ_y+1)*structor_base_len+hole_to_square])
							circle(r=frez_rad);
			
						translate([(2*sQ_x)*structor_base_len-hole_to_square, (2*sQ_y+1)*structor_base_len+hole_to_square])
							circle(r=frez_rad);
					}
				}
			if (x>1 && y>1) {
				for (sQ_x=[0:x-2]) {
					for (sQ_y = [0:y-2]) {
						translate([structor_base_len*(2*sQ_x+1), structor_base_len*(2*sQ_y+1)])
							square(structor_base_len);
					}
				}
				if (is_hole_rad) {
					for (sQ_x=[0:x-1]) {
						for (sQ_y = [0:y-1]) {
							translate([structor_base_len*(2*sQ_x+0.5), structor_base_len*(2*sQ_y+0.5)])
								circle(r=hole_rad); 
						}
					}
				}
			} else {
				if (x==1) {
					for (sQ_y = [0:y-1]) {
						translate([0.5*structor_base_len, structor_base_len*(2*sQ_y+0.5)])
							circle(r=hole_rad); 
					}
				}
				if (y==1) {
					for (sQ_x = [0:x-1]) {
						translate([structor_base_len*(2*sQ_x+0.5),0.5*structor_base_len])
							circle(r=hole_rad); 
					}
				}
	
		
			}
		}        
	}
}

module structor3d(dim, center=false, is_hole_rad=false) {
	linear_extrude(structor_base_len) structor2d(dim, center, is_hole_rad);
}

//structor3d([5,3], false, true);
//structor3d([2,12*2], false, true);
