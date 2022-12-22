let s:cpo_save=&cpo
set cpo&vim
imap <M-Bslash> <Plug>(copilot-suggest)
imap <M-[> <Plug>(copilot-previous)
imap <M-]> <Plug>(copilot-next)
imap <Plug>(copilot-suggest) <Cmd>call copilot#Suggest()
imap <Plug>(copilot-previous) <Cmd>call copilot#Previous()
imap <Plug>(copilot-next) <Cmd>call copilot#Next()
imap <Plug>(copilot-dismiss) <Cmd>call copilot#Dismiss()
imap <expr> <S-Tab> vsnip#jumpable(-1)  ? '<Plug>(vsnip-jump-prev)'      : '<S-Tab>'
inoremap <C-W> u
inoremap <C-U> u
smap <expr> 	 vsnip#jumpable(1)   ? '<Plug>(vsnip-jump-next)'      : '	'
nnoremap <silent>  :nohl
vnoremap  :'<,'>!sh -c "$(head -n1)"
nnoremap <silent>  o :SymbolsOutline
xnoremap # y?\V"
omap <silent> % <Plug>(MatchitOperationForward)
xmap <silent> % <Plug>(MatchitVisualForward)
nmap <silent> % <Plug>(MatchitNormalForward)
nnoremap & :&&
xnoremap * y/\V"
nnoremap Y y$
omap <silent> [% <Plug>(MatchitOperationMultiBackward)
xmap <silent> [% <Plug>(MatchitVisualMultiBackward)
nmap <silent> [% <Plug>(MatchitNormalMultiBackward)
nnoremap \dr :GdbStartRR
nnoremap \db :GdbStartBashDB bashdb main.sh
nnoremap \dp :GdbStartPDB python -m pdb main.py
nnoremap \dl :GdbStartLLDB lldb
nnoremap \dd :GdbStart gdb -q 
omap <silent> ]% <Plug>(MatchitOperationMultiForward)
xmap <silent> ]% <Plug>(MatchitVisualMultiForward)
nmap <silent> ]% <Plug>(MatchitNormalMultiForward)
xmap a% <Plug>(MatchitVisualTextObject)
xmap gx <Plug>NetrwBrowseXVis
nmap gx <Plug>NetrwBrowseX
omap <silent> g% <Plug>(MatchitOperationBackward)
xmap <silent> g% <Plug>(MatchitVisualBackward)
nmap <silent> g% <Plug>(MatchitNormalBackward)
xnoremap <silent> <Plug>NetrwBrowseXVis :call netrw#BrowseXVis()
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#BrowseX(netrw#GX(),netrw#CheckIfRemote(netrw#GX()))
xmap <silent> <Plug>(MatchitVisualTextObject) <Plug>(MatchitVisualMultiBackward)o<Plug>(MatchitVisualMultiForward)
onoremap <silent> <Plug>(MatchitOperationMultiForward) :call matchit#MultiMatch("W",  "o")
onoremap <silent> <Plug>(MatchitOperationMultiBackward) :call matchit#MultiMatch("bW", "o")
xnoremap <silent> <Plug>(MatchitVisualMultiForward) :call matchit#MultiMatch("W",  "n")m'gv``
xnoremap <silent> <Plug>(MatchitVisualMultiBackward) :call matchit#MultiMatch("bW", "n")m'gv``
nnoremap <silent> <Plug>(MatchitNormalMultiForward) :call matchit#MultiMatch("W",  "n")
nnoremap <silent> <Plug>(MatchitNormalMultiBackward) :call matchit#MultiMatch("bW", "n")
onoremap <silent> <Plug>(MatchitOperationBackward) :call matchit#Match_wrapper('',0,'o')
onoremap <silent> <Plug>(MatchitOperationForward) :call matchit#Match_wrapper('',1,'o')
xnoremap <silent> <Plug>(MatchitVisualBackward) :call matchit#Match_wrapper('',0,'v')m'gv``
xnoremap <silent> <Plug>(MatchitVisualForward) :call matchit#Match_wrapper('',1,'v'):if col("''") != col("$") | exe ":normal! m'" | endifgv``
nnoremap <silent> <Plug>(MatchitNormalBackward) :call matchit#Match_wrapper('',0,'n')
nnoremap <silent> <Plug>(MatchitNormalForward) :call matchit#Match_wrapper('',1,'n')
tnoremap <silent> <Plug>(fzf-normal) 
tnoremap <silent> <Plug>(fzf-insert) i
nnoremap <silent> <Plug>(fzf-normal) <Nop>
nnoremap <silent> <Plug>(fzf-insert) i
snoremap <expr> <BS> ("\<BS>" . (&virtualedit ==# '' && getcurpos()[2] >= col('$') - 1 ? 'a' : 'i'))
smap <expr> <S-Tab> vsnip#jumpable(-1)  ? '<Plug>(vsnip-jump-prev)'      : '<S-Tab>'
nnoremap <silent> <C-L> :nohl
imap <expr> 	 vsnip#jumpable(1)   ? '<Plug>(vsnip-jump-next)'      : '	'
inoremap  u
inoremap  u
let &cpo=s:cpo_save
unlet s:cpo_save
set completeopt=menu,menuone,noselect
set helplang=en
set listchars=tab:¸\ ,trail:·
set runtimepath=~/.config/nvim,~/.config/nvim/plugged/nvim-lsp-installer,~/.config/nvim/plugged/nvim-lspconfig,~/.config/nvim/plugged/cmp-nvim-lsp,~/.config/nvim/plugged/cmp-buffer,~/.config/nvim/plugged/cmp-path,~/.config/nvim/plugged/cmp-cmdline,~/.config/nvim/plugged/nvim-cmp,~/.config/nvim/plugged/cmp-vsnip,~/.config/nvim/plugged/vim-vsnip,~/.config/nvim/plugged/vim-vsnip-integ,~/.config/nvim/plugged/vim-templates,~/.config/nvim/plugged/vim-airline,~/.config/nvim/plugged/vim-airline-themes,~/.config/nvim/plugged/nvim-gdb,~/.config/nvim/plugged/popfix,~/.config/nvim/plugged/nvim-lsputils,~/.config/nvim/plugged/editorconfig-vim,~/.config/nvim/plugged/nvim-treesitter,~/.config/nvim/plugged/symbols-outline.nvim,~/.config/nvim/plugged/copilot.vim,~/.config/nvim/plugged/nvim-platformio,~/.config/nvim/plugged/nvim-navic,~/.config/nvim/plugged/indent-o-matic,/etc/xdg/nvim,~/.local/share/nvim/site,~/.local/share/flatpak/exports/share/nvim/site,/var/lib/flatpak/exports/share/nvim/site,/usr/local/share/nvim/site,/usr/share/nvim/site,/usr/share/nvim/runtime,/usr/share/nvim/runtime/pack/dist/opt/matchit,/usr/lib/nvim,/usr/share/nvim/site/after,/usr/local/share/nvim/site/after,/var/lib/flatpak/exports/share/nvim/site/after,~/.local/share/flatpak/exports/share/nvim/site/after,~/.local/share/nvim/site/after,/etc/xdg/nvim/after,~/.config/nvim/plugged/cmp-nvim-lsp/after,~/.config/nvim/plugged/cmp-buffer/after,~/.config/nvim/plugged/cmp-path/after,~/.config/nvim/plugged/cmp-cmdline/after,~/.config/nvim/plugged/cmp-vsnip/after,~/.config/nvim/plugged/vim-vsnip-integ/after,~/.config/nvim/after
set shiftwidth=2
set softtabstop=2
set statusline=%{%v:lua.require'nvim-navic'.get_location()%}
set tabstop=2
set window=48
" vim: set ft=vim :
