% Shim optimization code
%
% Author: Trong-Kha Truong (trongkha.truong@duke.edu)
% Copyright (c) 2022 Duke University
%
% input:
%   B0_basis:   basis B0 maps
%   B0_toshim:  B0 map before shimming
%   mask:       shimming ROI (1 inside, NaN outside)
%   DCmax:      maximum current amplitude
% output:
%   DC:         optimal currents
%   B0_shimmed: B0 map after shimming

function [DC,B0_shimmed] = shim_code(B0_basis,B0_toshim,mask,DCmax)

[nx,ny,nz,nc] = size(B0_basis);
opt = optimoptions('fmincon','MaxIter',3000,'MaxFunEvals',20000,...
  'TolX',1e-10,'Tolfun',1e-9);
DC = fmincon(@(X) fun(X,B0_basis,B0_toshim,mask),zeros(1,nc),[],[],[],[],...
  -DCmax*ones(1,nc),DCmax*ones(1,nc),[],opt);
B0_coils = B0_basis .* repmat(reshape(DC,[1 1 1 nc]),[nx ny nz 1]);
B0_shimmed = (B0_toshim + sum(B0_coils,4)) .* mask;

function RMSE = fun(X,B0_basis,B0_toshim,mask)

[nx,ny,nz,nc] = size(B0_basis);
B0_coils = B0_basis .* repmat(reshape(X,[1 1 1 nc]),[nx ny nz 1]);
B0_shimmed = (B0_toshim + sum(B0_coils,4)) .* mask;
RMSE = sqrt(nanmean(B0_shimmed(:).^2));
