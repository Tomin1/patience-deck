; Just a little testing "game" for rng state
; Copyright (C) 2024-2025 Tomi Leppänen
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, version 3 of the License.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.

(use-modules (aisleriot interface) (aisleriot api))

(define stock 0)
(define waste 1)

(define (new-game)
  (initialize-playing-area)
  (set-ace-low)

  (make-standard-deck)
  (shuffle-deck)

  (add-normal-slot DECK 'stock)
  (add-normal-slot '() 'waste)

  (list 2 1))

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
  (or (and (= slot-id stock)
           (flip-stock stock waste -1))
      (and (= slot-id waste)
           (shuffle-slot stock))))

(define (button-double-clicked slot-id)
  #f)

(define (game-continuable)
  #t)

(define (game-won)
  #f)

(define (get-hint)
  #f)

(define (get-options)
  #f)

(define (apply-options options)
  #f)

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
