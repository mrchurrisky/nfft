% Test script of class nfft for spatial dimension d=1.
clear all;
addpath('../');
nfftdir=findnfftdir();
addpath(strcat(nfftdir,'/matlab/nfft'));

M=16; % number of nodes
N=24; % number of Fourier coefficients in first direction

x=rand(M,1)-0.5; %nodes

% Initialisation
plan=nfft(1,N,M); % create plan of class type nfft
%n=2^(ceil(log(N)/log(2))+1);
%plan=nfft(1,N,M,n,7,'PRE_PHI_HUT','FFTW_MEASURE'); % use of nfft_init_guru

plan.x=x; % set nodes in plan
nfft_precompute_psi(plan); % precomputations

% NFFT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

fhat=rand(N,1); % Fourier coefficients
fhatv=fhat(:);

% Compute samples with NFFT
plan.fhat=fhatv; % set Fourier coefficients
nfft_trafo(plan); % compute nonequispaced Fourier transform
f1=plan.f; % get samples

% Compute samples direct
k1=(-N/2:N/2-1).';
f2=zeros(M,1);
for j=1:M
	x1j=x(j,1);
	f2(j)=sum( fhatv.*exp(-2*pi*1i*k1*x1j) );
end %for

% Compare results
max(abs(f1-f2))

% Adjoint NFFT %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Computation with NFFT
nfft_adjoint(plan);
fhat1=plan.fhat;

% Direct computation
fhat2=zeros(N,1);
for j=1:N
	k1j=k1(j);
	fhat2(j)=sum( plan.f.*exp(2*pi*1i*k1j*x(:,1)) );
end %for

% Compare results
max(abs(fhat1-fhat2))

