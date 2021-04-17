; Just a little testing "game" for options.
; Copyright (C) 2021 Tomi Leppänen
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.

(use-modules (aisleriot interface) (aisleriot api))

(define option-one #t)
(define option-two #f)
(define option-three #f)
(define option-four #f)
(define option-five #t)
(define option-six #f)
(define option-A #t)
(define option-B #f)
(define option-C #f)
(define option-D #t)

(define (new-game)
  (initialize-playing-area)

  (add-normal-slot '() 'waste)

  (list 1 1))

(define (button-pressed slot-id card-list)
  #f)

;;; Not essential, see set-features below.
(define (droppable? start-slot card-list end-slot)
  #f)

(define (button-released start-slot card-list end-slot)
  ;; This will often start with somthing like:
  ;;  (if (droppable? start-slot card-list end-slot ...
  #f)

(define (button-clicked slot-id)
  #f)

(define (button-double-clicked slot-id)
  #f)

(define (game-continuable)
  #t)

(define (game-won)
  #f)

(define (get-hint)
  #f)

(define (get-options) 
  (list (list (_"Option A") option-A)
        'begin-exclusive 
	(list (_"Option One") option-one)
	(list (_"Option Two") option-two)
	(list (_"Option Three") option-three)
        'end-exclusive
	(list (_"Option B") option-B)
	(list (_"Option C") option-C)
        'begin-exclusive 
	(list (_"Option Four") option-four)
	(list (_"Option Five") option-five)
	(list (_"Option Six") option-six)
        'end-exclusive
	(list (_"Option D") option-D)))

(define (apply-options options) 
  (set! option-A (cadr (list-ref options 0)))
  (set! option-one (cadr (list-ref options 2)))
  (set! option-two (cadr (list-ref options 3)))
  (set! option-three (cadr (list-ref options 4)))
  (set! option-B (cadr (list-ref options 6)))
  (set! option-C (cadr (list-ref options 7)))
  (set! option-four (cadr (list-ref options 9)))
  (set! option-five (cadr (list-ref options 10)))
  (set! option-six (cadr (list-ref options 11)))
  (set! option-D (cadr (list-ref options 13))))

(define (timeout) 
  #f)

(define (dealable?)
  #f)

(define (do-deal-next-cards)
  #f)

;; Define the optional features the game uses. Valid options are:
;;  droppable-feature: An predicate, droppable?, is defined that will 
;;                     return whether the card can be dropped here. 
;;                     This is used by the drawing code to highlight
;;                     droppable locations.
;;  dealable-feature: An predicate, dealable?, is defined that will
;;                    return whether new card(s) can be dealt.
(set-features droppable-feature dealable-feature)

(set-lambda new-game button-pressed button-released button-clicked
button-double-clicked game-continuable game-won get-hint get-options
apply-options timeout droppable? dealable?)
