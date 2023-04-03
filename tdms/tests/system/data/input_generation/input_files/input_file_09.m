%specify the interpolation method (1 or not present - cubic, 2 - bandlimited)
intmethod = 1;
%specify the solver method (1 or not present - FDTD, 0 - PSTD)
usecd = 0;
%whether we are using a compact source condition
compactsource = 1;

%these are not involved in the formal input file spec
lambda = 1300e-9;

%size of Yee cell in metres
delta.x = lambda/6;
delta.y = lambda/6;
delta.z = lambda/6;


%define the grid size, a square of side 1.5 wavelengths
I = 256;
J = 256;
K = 32;
%K = 5500;

%order of the PML conductivity profile curve
n = 4;

%maximum reflection at PML
R0 = 1e-7;

%number of PML cells in each direction
Dxl = 10;
Dxu = 10;
Dyl = 10;
Dyu = 10;
Dzl = 10;
Dzu = 10;

%courant time step
dt = 2/sqrt(3)/pi*delta.x/(3e8/1.35)*.95;

%define the number of time steps
Nt = 50; %set for quick test to generate algorithmically correct
         %(physically incorrect) results
%Nt = 1000; %required to generate physically correct results

%water
epsr = 1.35^2;
mur = 1;
kappa_max = 1;
multilayer = [];

%frequency in Hz
f_an = asin( 2*pi/1300e-9*2.997924580105029e+08*dt/2)/(pi*dt);

%This is where we define the planes where the incident waveforms
%are introduced. The variable interface has 6 members, I0, I1, J0,
%J1, K0, K1 which are 1x2 vectors. The first entry is the position
%of the plane, in local coordinates and the second entry os whether
%or not to apply the interface condition at that plane
interface.I0 = [5 0];
interface.I1 = [I-5 0];
interface.J0 = [5 0];
interface.J1 = [J-5 0];
interface.K0 = [10 1];
interface.K1 = [K-5 0];

%not used as run mode is complete
outputs_array ={};

%these are the function names used to generate the field
efname = 'efield_plane';
hfname = '';%'hfield_focused_equiv';

%this is the z value at which the field is launched, in metres
z_launch = 0;


%this defines the point about which the illumination is centred in
%the so called 'interior' coordinate system
illorigin = [floor(I/2) floor(J/2) 20];

%the wavelength width (in m). This corresponds to the FWHM of the
wavelengthwidth = 120e-9;

%this defines the run mode of the simulation, can be 'analyse' or
%'complete'. 'analyse' means that sub results can be saved using
%the statements in outputs_array. When complete is specified, only
%the final results will be saved using the outputs_array statements.
%runmode = 'analyse';
runmode = 'complete';


%this is the kind of source mode, can be 'steadystate' or
%sourcemode = 'steadystate';
sourcemode = 'pulsed';

%this determines whether or not to extract phasors in the volume of
%the grid
exphasorsvolume = 0;

%this determines whether or not to extract phasors around a
%specified surface
exphasorssurface = 0;

%this specifies a surface to extract the phasors at. These
%quantities are in interior coordinate system;
%has the form [I0 I1 J0 J1 K0 K1] which defines the extremes of a
%cuboid wihch defines the surface to extract phasors at
%These should be set so that the interpolation scheme can work
phasorsurface = [5 I-5 5 J-5 5 K-5];

%could be '3' 'TE' or 'TM'
dimension = '3';

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
fieldsample.i = [];
fieldsample.j = [];
fieldsample.k = [];
fieldsample.n = [];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
[ii,jj,kk] = ndgrid((I/2-4):(I/2+4),(J/2-4):(J/2+4),(K/2-4):(K/2+4));
campssample.vertices = [];
campssample.components = [];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Nlambda = 4;
lambda0 = 1300e-9;
dlambda = 170e-9;
b = 4*sqrt(log(2))*lambda0^2/(2*pi*3e8*dlambda);
omega0 = 2*pi*3e8/lambda0;
%omega1 = 2*pi*3e8/(lambda0-dlambda/2);

omega_min = omega0 - sqrt(4/b^2*log(10^3));
omega_max = omega0 + sqrt(4/b^2*log(10^3));

lambda_min = 3e8*2*pi/omega_max;
lambda_max = 3e8*2*pi/omega_min;


omega_vec = linspace(omega_min,omega_max,Nlambda);
k_vec = omega_vec/2.997924580105029e+08;
%k_vec = omega0/2.997924580105029e+08;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

f_ex_vec = asin( k_vec*2.997924580105029e+08*dt/2)/(pi*dt);

%ignore all below here
exdetintegral=1;
k_det_obs=10;
%k_obs = k_det_obs;
NA_det=0.1;
%NA = NA_det;
beta_det=25/36;
detmodevec=1:31;
detsensefun='gaussian_detfun';
air_interface = [];
