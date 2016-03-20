set number                                                                                                                                                   
syntax on                                                                                                                                                     
set background=dark                                                                                                                                           
set nocompatible                                                                                                                                              
set autoindent                                                                                                                                                
set smartindent                                                                                                                                               
set tabstop=8        " tab width is 8 spaces                                                                                                                  
set shiftwidth=8     " indent also with 8 spaces                                                                                                              
set comments=sl:/*,mb:\ *,elx:\ */

" 80 characters line                                                                                                                                          
set colorcolumn=81                                                                                                                                            
execute "set colorcolumn=" . join(range(81,335), ',')                                                                                                         
highlight ColorColumn ctermbg=Black ctermfg=DarkRed

" Highlight trailing spaces                                                                                                                                   
highlight ExtraWhitespace ctermbg=red guibg=red                                                                                                               
match ExtraWhitespace /\s\+$/                                                                                                                                 
autocmd BufWinEnter * match ExtraWhitespace /\s\+$/                                                                                                           
autocmd InsertEnter * match ExtraWhitespace /\s\+\%#\@<!$/                                                                                                    
autocmd InsertLeave * match ExtraWhitespace /\s\+$/                                                                                                           
autocmd BufWinLeave * call clearmatches()
